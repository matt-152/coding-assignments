/* Matthew Delaney System Admin Application
 *                                                                              
 * Using C as I'm more familiar with it. I wouldn't know how to use C++'s
 * extended feature set to improve this program, so for the sake of honesty I'll
 * just keep it plain C. I commented this code excessively just to show my
 * thought process - the only comment I'd include if this were a real project
 * would be the header at the bottom. Filtering the comments out might give you
 * a better idea on how I write code.
 *
 * This code was written in vim on the terminal. I use tmux to split the window
 * into three panes. The top left runs vim, the top right is running aichat
 * where I can consult gpt-4.1 for any questions, the bottom pane runs an entr
 * command which builds and runs my code on every write, so I can easily monitor
 * my progress.
 * 
 * My general approach for a simple problem like this is to rough out a
 * functional prototype before refactoring for clarity. In this case everything
 * started out crammed into main(), then I started sliding variables and
 * functions out, eliminated a redudancy here and there, until I arrived at the
 * final result.
 *
 * Normally when writing code I try to practice test-driven-development, but for
 * this simple example, and since only one file can be uploaded, I decided
 * against it.
 *
 * I tried to make the implementation readable, though I understand that's
 * subjective. Here's my approach: I write in "Table of Contents" style. At the
 * top in the main method is the central logic or algorithm for the file, in as
 * plain-english a form as I can make it, by breaking out the lower level code
 * into functions with meaningful names. I try to declare functions in the order
 * they are used. I try to keep the file "flat"; that is, the main method will
 * call lower-level functions, but I try to avoid having those functions call
 * yet other functions, as to reduce the amount of jumping around needed to
 * understand a file. I think of the main method as acting like the "table of
 * contents" for the rest of the file, hence, "Table of Contents" style. 
 *
 * One caveat with this. In C, functions must be declared before they are used,
 * so the code can't be easily written top-to-bottom. I work around this by
 * simply writing bottom-to-top. The main function is at the bottom, the first
 * function it calls its second-from-bottom, etc. For this reason, when reading
 * this example, it might be more comprehendable if you start from the bottom
 * and go up.
 */

#include <stdio.h>
#include <stdlib.h>

FILE *input_file = NULL;
FILE *output_file = NULL;

char *input_line_buffer = NULL;
size_t input_line_size = 0;

// With this function you see one downside in breaking apart main() into
// separate functions. With everything all in one place, checks for whether the
// input file was loaded properly can be run once. When lines like this are
// isolated in their own function, the abstraction can hide potentially
// dangerous pitfalls, like calling fprintf without initializing all the files.
// So you either need to include that check in the function itself, and have it
// run every time its called, or leave a potential landmine for someone to step
// on.
// My bias is towards readability, but the tradeoffs between efficiency, safety,
// and readability can only be determined when working with a real project.
void output_line(void) {
    if (input_line_buffer && output_file)
        fprintf(output_file, "%s", input_line_buffer);
}

// The heart of this little example. I used pointer arithmetic to keep this
// particularly concise. It might be a bit too clever though.
// Note the "while (*right" loop. I have to scan for the end of the line,
// starting from the beginning, since input_buffer_size is just the size of the
// allocated buffer, which is the smallest factor of 8 bytes that will fit the
// line. Scanning from the end of that will mean you're scanning inintialized
// memory, which is unpredicatble. Spent quite a bit of time figuring that
// little bug out.
void reverse_line(void) {
    char *left = input_line_buffer;

    char *right = left;
    while (*right != '\n') right++;
    right--;

    char tmp;

    while (left < right) {
        tmp = *left; *left = *right; *right = tmp;
        left++; right--;
    }
}

#define END_OF_INPUT (-1)

int get_next_line(void) {
    if (input_file) 
        return getline(&input_line_buffer, &input_line_size, input_file);
    return END_OF_INPUT;
}

void cleanup(void) {
    if (input_file) fclose(input_file);
    if (output_file) fclose(output_file);
    if (input_line_buffer) free(input_line_buffer);
}

#define EXIT_ERR cleanup(); exit(1);
#define EXIT_SUCC cleanup(); exit(0);

// This function is neat because it emerged as a result of refactoring.
// In the rough draft I just had it in the main function to open the input file,
// check for success, then open the output file and check. If the output failed
// to open, I made sure to close the input file. Thus they were different enough
// that I wondered whether it'd even be worthwhile to try and combine them into
// a single function, despite how similar they were. After creating the above
// cleanup function for a different reason, I found I was now able to write a
// generic method easily. I made sure ptr was the first argument - by looking at
// how this function is used in main you can see how this prevents this generic
// function from making the code less readable.
void open(FILE **ptr, char *filename, char *mode) {
    *ptr = fopen(filename, mode);
    if (!*ptr) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        EXIT_ERR;
    }
}

void print_usage(char *program) {
    fprintf(stderr, "Usage: %s [in-file] [out-file]\n", program);
}

int main(int argc, char *argv[]) {
    // This seemingly pointless variable serves an explanatory purpose. While
    // you could easily infer that the reason we're checking argc is to validate
    // the given arguments, I think minimizing that mental jugglery really adds
    // up across an entire codebase. Note too how it communicates something
    // subtle about this program's correctness. There's an amount of dissonance
    // on reading that all we check to make sure the arguments are correct is
    // whether or not there's 3 of them. Argument handling clearly isn't very
    // robust and might be worth revisting later.
    _Bool args_correct = argc == 3;
    if (! args_correct) {
        print_usage(argv[0]);
        EXIT_ERR;
    }

    // Thought open() was clever when I first wrote it, now I wonder if it's
    // misleading. We're technically opening the provided filename here,
    // input_file and output_file just store the result of that operation
    open(&input_file, argv[1], "r");
    open(&output_file, argv[2], "w");

    // I come from a non-C programming background, so combining the stateful
    // operation of getting the next line with checking an end-of-input
    // condition still kind of bothers me. It's a common C idiom though, and it
    // fixes a particularly tricky bug I spent much too long trying to
    // fix after writing it the non-C way
    while (get_next_line() != END_OF_INPUT) {
        reverse_line();
        output_line();
    }

    EXIT_SUCC;
}

// Below is this code's "header". A short blurb about what this file does and why
// it exists. If this were part of a larger project, I'd also have a sentance or
// two explaining how this file's function fit into the whole. My hope is
// comments like this can act as signposts, decreasing the effort needed to
// navigate through a code base.
// 
// Some people say that comments should be avoided, since they're fragile and can
// quickly become outdated and misleading.  In my view, coarser-grained
// "structural" comments like this avoid these problems while communicating
// information that's very hard to communicate in the code itself.
// 
// As explained elsewhere, the "header" is at the bottom since C encourages a
// bottom-up style. Putting this info at the bottom has the benefit of putting
// the file's purpose and highest-level function next to each other, where they
// can inform each others meaning. Plus, if you wanted to get an overview of
// many such files, you could just `tail` them and review the output.


/* The code in this file compiles to an executable which takes an input filename
 * and an output filename. It reads each line in the input file, reverses it,
 * and writes it to the output file.
 *
 * This code exists as part of the application process for a Quantiq Partners
 * position. I am applying for the System Administrator role.
 */
