/*
 * Copyright 2008-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>

#include <print/esc.h>

#include <adt/set.h>

#include <fsm/fsm.h>
#include <fsm/pred.h>
#include <fsm/walk.h>
#include <fsm/print.h>
#include <fsm/options.h>

#include "libfsm/internal.h"

#include "ir.h"

static unsigned int
ir_indexof(const struct ir *ir, const struct ir_state *cs)
{
	assert(ir != NULL);
	assert(cs != NULL);
	assert(cs >= ir->states);

	return cs - ir->states;
}


static int
leaf(FILE *f, const void *state_opaque, const void *leaf_opaque)
{
	assert(f != NULL);
	assert(leaf_opaque == NULL);

	(void) state_opaque;
	(void) leaf_opaque;

	/* XXX: this should be FSM_UNKNOWN or something non-EOF,
	 * maybe user defined */
	fprintf(f, "state := known;");

	return 0;
}

static void
print_ranges(FILE *f, const struct ir *ir, const struct fsm_options *opt,
	const struct ir_range *ranges, size_t n, unsigned int step, int following)
{
	size_t k;
	size_t c;

	assert(f != NULL);
	assert(ir != NULL);
	assert(opt != NULL);
	assert(ranges != NULL);

	for (k = 0; k < n; k++)
	{
		assert(ranges[k].end >= ranges[k].start);

		for (c = ranges[k].start; c <= ranges[k].end; c++)
		{
			if (!following && k == 0)
				fprintf(f, "\t\tif    inp = x\"%x\" then state := S%d;", c, step);
			else
				fprintf(f, "\t\telsif inp = x\"%x\" then state := S%d;", c, step);

			fprintf(f, "\n");
		}
	}
}

static void
print_groups(FILE *f, const struct ir *ir, const struct fsm_options *opt,
	unsigned csi,
	const struct ir_group *groups, size_t n)
{
	size_t j;

	assert(f != NULL);
	assert(ir != NULL);
	assert(opt != NULL);
	assert(groups != NULL);

	for (j = 0; j < n; j++) {
		assert(groups[j].ranges != NULL);
		print_ranges(f, ir, opt, groups[j].ranges, groups[j].n, groups[j].to, j);
	}
	fprintf(f, "\t\telse                   state := unknown;\n");
	fprintf(f, "\t\tend if;\n");
}

static void
print_singlecase(FILE *f, const struct ir *ir, const struct fsm_options *opt,
	const char *cp,
	struct ir_state *cs,
	int (*leaf)(FILE *, const void *state_opaque, const void *leaf_opaque),
	const void *leaf_opaque)
{
	assert(ir != NULL);
	assert(opt != NULL);
	assert(cp != NULL);
	assert(f != NULL);
	assert(cs != NULL);
	assert(leaf != NULL);

	switch (cs->strategy) {
	case IR_TABLE:
		/* TODO */
		abort();

	case IR_NONE:
		fprintf(f, "\t\t");
		leaf(f, cs->opaque, leaf_opaque);
		fprintf(f, "\n");
		return;

	case IR_SAME:
		fprintf(f, "\t\t\t");
		if (cs->u.same.to != ir_indexof(ir, cs)) {
			fprintf(f, "state = S%u; ", cs->u.same.to);
		}
		fprintf(f, "break;\n");
		return;

	case IR_COMPLETE:
		fprintf(f, "\t\t\tswitch ((unsigned char) %s) {\n", cp);

		print_groups(f, ir, opt, ir_indexof(ir, cs), cs->u.complete.groups, cs->u.complete.n);

		fprintf(f, "\t\t\t}\n");
		fprintf(f, "\t\t\tbreak;\n");
		return;

	case IR_PARTIAL:

		print_groups(f, ir, opt, ir_indexof(ir, cs), cs->u.partial.groups, cs->u.partial.n);

		return;

	case IR_DOMINANT:
		fprintf(f, "\t\t\tswitch ((unsigned char) %s) {\n", cp);

		print_groups(f, ir, opt, ir_indexof(ir, cs), cs->u.dominant.groups, cs->u.dominant.n);

		fprintf(f, "\t\t\tdefault: ");
		if (cs->u.dominant.mode != ir_indexof(ir, cs)) {
			fprintf(f, "state = S%u; ", cs->u.dominant.mode);
		}
		fprintf(f, "break;\n");

		fprintf(f, "\t\t\t}\n");
		fprintf(f, "\t\t\tbreak;\n");
		return;

	case IR_ERROR:
		fprintf(f, "\t\t\tswitch ((unsigned char) %s) {\n", cp);

		print_groups(f, ir, opt, ir_indexof(ir, cs), cs->u.error.groups, cs->u.error.n);

		print_ranges(f, ir, opt, cs->u.error.error.ranges, cs->u.error.error.n, 0, 0);
		fprintf(f, " ");
		leaf(f, cs->opaque, leaf_opaque);
		fprintf(f, "\n");

		fprintf(f, "\t\t\tdefault: ");
		if (cs->u.error.mode != ir_indexof(ir, cs)) {
			fprintf(f, "state = S%u; ", cs->u.error.mode);
		}
		fprintf(f, "break;\n");

		fprintf(f, "\t\t\t}\n");
		fprintf(f, "\t\t\tbreak;\n");
		return;
	}

	fprintf(f, "\t\t\tswitch ((unsigned char) %s) {\n", cp);

	fprintf(f, "\t\t\t}\n");

	fprintf(f, "\t\t\tbreak;\n");
}

static void
print_stateenum(FILE *f, size_t n)
{
	size_t i;
	assert(f != NULL);

	fprintf(f, "  type states is (unknown, known, ");

	for (i = 0; i < n; i++) {
		fprintf(f, "S%u", (unsigned) i);
		if (i + 1 < n) {
			fprintf(f, ", ");
		}

		if (i + 1 < n && (i + 1) % 10 == 0) {
			fprintf(f, "\n");
			fprintf(f, "\t\t");
		}
	}

	fprintf(f, ");\n");
}

int
fsm_print_vhdlfrag(FILE *f, const struct ir *ir, const struct fsm_options *opt,
	const char *cp,
	int (*leaf)(FILE *, const void *state_opaque, const void *leaf_opaque),
	const void *leaf_opaque)
{
	unsigned i;

	assert(f != NULL);
	assert(ir != NULL);
	assert(opt != NULL);
	assert(cp != NULL);

	for (i = 0; i < ir->n; i++) {
		fprintf(f, "\twhen S%u =>", i);

		if (opt->comments) {
			if (ir->states[i].example != NULL) {
				fprintf(f, " -- e.g. \"");
				escputs(f, opt, c_escputc_str, ir->states[i].example);
				fprintf(f, "\"");
			} else if (i == ir->start) {
				fprintf(f, " -- start");
			}
		}
		fprintf(f, "\n");
		print_singlecase(f, ir, opt, cp, &ir->states[i], leaf, leaf_opaque);
		fprintf(f, "\n");
	}
	return 0;
}


void
fsm_print_vhdl(FILE *f, const struct fsm *fsm)
{
	struct ir *ir;
	const char *prefix;

	assert(f != NULL);
	assert(fsm != NULL);
	assert(fsm->opt != NULL);

	ir = make_ir(fsm);
	if (ir == NULL) {
		return;
	}

	/* print head */
	fprintf(f, "--Standard Library \n");
	fprintf(f, "library ieee; \n");
	fprintf(f, "use ieee.std_logic_1164.all;\n");
	fprintf(f, "use ieee.numeric_std.all;\n");
	fprintf(f, "use std.textio.all; -- Imports the standard textio package.\n");
	fprintf(f, "\n");
	fprintf(f, "entity fsm is\n");
	fprintf(f, "  port (\n");
	fprintf(f, "    clk  : in   std_logic;\n");
	fprintf(f, "    rst  : in   std_logic;\n");
	fprintf(f, "    inp  : in   std_logic_vector(7 downto 0);\n");
	fprintf(f, "    lock : out  std_logic;\n");
	fprintf(f, "    oot  : out  std_logic\n");
	fprintf(f, "  );\n");
	fprintf(f, "end fsm;\n");
	fprintf(f, "\n");
	fprintf(f, "architecture rtl of fsm is\n");
	fprintf(f, "begin\n");
	fprintf(f, "  p_sync_fsm : process\n");
	fprintf(f, "  variable l : line;    \n");

	print_stateenum(f, ir->n);

  	fprintf(f, "  variable state: states := S0;\n");
  	fprintf(f, "  begin\n");
  	fprintf(f, "    wait until rising_edge(clk);\n");
  	fprintf(f, "    write (l, String'(\"fsm: clock\")); writeline (output, l);\n");
  	fprintf(f, "\n");
  	fprintf(f, "    case state is\n");

	(void)fsm_print_vhdlfrag(f, ir, fsm->opt, fsm->opt->cp,
						  fsm->opt->leaf != NULL ? fsm->opt->leaf : leaf, fsm->opt->leaf_opaque);

	/* tail */
	fprintf(f, "\twhen known =>\n");
	fprintf(f, "\t\tstate := S0;\n");
	fprintf(f, "\n");
	fprintf(f, "\twhen unknown =>\n");
	fprintf(f, "\t\tstate := S0;\n");
	fprintf(f, "    end case;\n");
	fprintf(f, "\n");
	fprintf(f, "    -- restart if rst is on\n");
	fprintf(f, "    if (rst = '1') then\n");
	fprintf(f, "      write (l, String'(\"fsm: state reset\")); writeline (output, l);\n");
	fprintf(f, "      state := S0;\n");
	fprintf(f, "    end if;\n");
	fprintf(f, "\n");
	fprintf(f, "    -- set/rst lock\n");
	fprintf(f, "    if (state = S0) then\n");
	fprintf(f, "      write (l, String'(\"fsm: state S0\")); writeline (output, l);\n");
	fprintf(f, "      lock <= '0';\n");
	fprintf(f, "      oot <= '0';\n");
	fprintf(f, "    else\n");
	fprintf(f, "      lock <= '1';\n");
	fprintf(f, "    end if;\n");
	fprintf(f, "\n");
	fprintf(f, "    -- set output when we're done\n");
	fprintf(f, "    if (state = known) then\n");
	fprintf(f, "      write (l, String'(\"fsm: state known\")); writeline (output, l);\n");
	fprintf(f, "      oot <= '1';\n");
	fprintf(f, "      lock <= '0';\n");
	fprintf(f, "    elsif (state = unknown) then\n");
	fprintf(f, "      write (l, String'(\"fsm: state unknown\")); writeline (output, l);\n");
	fprintf(f, "      oot <= '0';\n");
	fprintf(f, "      lock <= '0';\n");
	fprintf(f, "    end if;\n");
	fprintf(f, "  end process p_sync_fsm;\n");
	fprintf(f, "end rtl;\n");

	free_ir(ir);
}

