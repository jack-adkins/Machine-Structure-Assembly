/****************************************************************************
 *             testing.c
 *
 * Assignment: um
 * Authors: Jack Adkins, Seth Gellman
 * NOTE: this was originally umlabwrite.c, so most of it was not written by us.
 * Date: 11/17/24
 *
 * Summary:
 * This file defines and sets up the overall unit testing framework, using
 * a struct array to create files for input and output to allow us to diff
 * test our universal machine with inputs and outputs that we make manually.
****************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "assert.h"
#include "fmt.h"
#include "seq.h"

extern void Um_write_sequence(FILE *output, Seq_T instructions);

extern void build_halt_test(Seq_T instructions);
extern void build_verbose_halt_test(Seq_T instructions);
extern void build_output_test(Seq_T stream);
extern void build_add_test(Seq_T stream);
extern void build_mult_test(Seq_T stream);
extern void build_div_test(Seq_T stream);
extern void build_multdiv_test(Seq_T stream);
extern void build_allmath_test(Seq_T stream);
extern void build_bnand_test(Seq_T stream);
extern void build_double_bnand_test(Seq_T stream);
extern void build_map_test(Seq_T stream);
extern void build_unmap_test(Seq_T stream);
extern void build_unmap_many_test(Seq_T stream);
extern void build_io_test(Seq_T stream);
extern void build_io_eof_test(Seq_T stream);
extern void build_cmov_execute_test(Seq_T stream);
extern void build_cmov_no_execute_test(Seq_T stream);
extern void build_lp_not0_test(Seq_T stream);
extern void build_lp_0_test(Seq_T stream);
extern void build_segload_test(Seq_T stream);
extern void build_mil50_test(Seq_T stream);
extern void build_sstore_test(Seq_T stream);
extern void build_sstore_0_test(Seq_T stream);
extern void build_long_test(Seq_T stream);
/* The array `tests` contains all unit tests for the lab. */

static struct test_info {
        const char *name;
        const char *test_input;          /* NULL means no input needed */
        const char *expected_output;
        /* writes instructions into sequence */
        void (*build_test)(Seq_T stream);
} tests[] = {
        { "halt",         NULL, "", build_halt_test },
        { "halt-verbose", NULL, "", build_verbose_halt_test },
        { "output-test", NULL, "g", build_output_test },
        {"add-test",  NULL, "6", build_add_test },
        {"mult-test",  NULL, "-", build_mult_test },
        {"div-test",  NULL, "(", build_div_test },
        {"mult-div-test",  NULL, "H", build_multdiv_test },
        {"allmath-test",  NULL, "4", build_allmath_test },
        {"bnand-test",  NULL, "O", build_bnand_test },
        {"double-bnand-test",  NULL, "a", build_double_bnand_test },
        {"map-test", NULL, "", build_map_test},
        {"unmap-test", NULL, "", build_unmap_test},
        {"unmap-many", NULL, "", build_unmap_many_test},
        {"io", "4", "4", build_io_test },
        {"io-eof", "8", "8K", build_io_eof_test},
        {"cmov-exec", NULL, ":", build_cmov_execute_test },
        {"cmov-no-exec", NULL, "d", build_cmov_no_execute_test },
        {"lp", NULL, "", build_lp_not0_test },
        {"lp-0seg", NULL, "", build_lp_0_test},
        {"segload", NULL, "3", build_segload_test },
        {"sstore", NULL, "", build_sstore_test},
        {"sstore-0", NULL, "", build_sstore_0_test },
        {"long-test", "?", "f3f?", build_long_test}
};

  
#define NTESTS (sizeof(tests)/sizeof(tests[0]))

/*
 * open file 'path' for writing, then free the pathname;
 * if anything fails, checked runtime error
 */
static FILE *open_and_free_pathname(char *path);

/*
 * if contents is NULL or empty, remove the given 'path', 
 * otherwise write 'contents' into 'path'.  Either way, free 'path'.
 */
static void write_or_remove_file(char *path, const char *contents);

static void write_test_files(struct test_info *test);


int main (int argc, char *argv[])
{
        bool failed = false;
        if (argc == 1)
                for (unsigned i = 0; i < NTESTS; i++) {
                        printf("***** Writing test '%s'.\n", tests[i].name);
                        write_test_files(&tests[i]);
                }
        else
                for (int j = 1; j < argc; j++) {
                        bool tested = false;
                        for (unsigned i = 0; i < NTESTS; i++)
                                if (!strcmp(tests[i].name, argv[j])) {
                                        tested = true;
                                        write_test_files(&tests[i]);
                                }
                        if (!tested) {
                                failed = true;
                                fprintf(stderr,
                                        "***** No test named %s *****\n",
                                        argv[j]);
                        }
                }
        return failed; /* failed nonzero == exit nonzero == failure */
}


static void write_test_files(struct test_info *test)
{
        FILE *binary = open_and_free_pathname(Fmt_string("%s.um", test->name));
        Seq_T instructions = Seq_new(0);
        test->build_test(instructions);
        Um_write_sequence(binary, instructions);
        Seq_free(&instructions);
        fclose(binary);

        write_or_remove_file(Fmt_string("%s.0", test->name),
                             test->test_input);
        write_or_remove_file(Fmt_string("%s.1", test->name),
                             test->expected_output);
}


static void write_or_remove_file(char *path, const char *contents)
{
        if (contents == NULL || *contents == '\0') {
                remove(path);
        } else {
                FILE *input = fopen(path, "wb");
                assert(input != NULL);

                fputs(contents, input);
                fclose(input);
        }
        free(path);
}


static FILE *open_and_free_pathname(char *path)
{
        FILE *fp = fopen(path, "wb");
        assert(fp != NULL);

        free(path);
        return fp;
}
