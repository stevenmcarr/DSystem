/* $Id: Options.h,v 1.5 1997/03/27 20:51:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
# ifndef options_h
# define options_h

# ifndef general_h
#include <libs/support/misc/general.h>
# endif
#include <libs/support/lists/SinglyLinkedList.h>

struct OptionsS {
  char *title;
  SinglyLinkedList olist;
  OptionsS(char *t) : title(t) {};
};

struct OptionsIteratorS {
  SinglyLinkedListIterator it;
  OptionsIteratorS(SinglyLinkedList *l) : it(l) {};
};



typedef FUNCTION_POINTER(void, OPT_FLAG_CLBK_FN, (void *handle));

typedef FUNCTION_POINTER(void, OPT_CHOICE_CLBK_FN, (void *handle, Generic d));

typedef FUNCTION_POINTER(void, OPT_STRING_CLBK_FN, (void *handle, char *s));

typedef enum {flag, choice, string} option_type;

struct flag_ {
  OPT_FLAG_CLBK_FN callback;
  char *name, *help;
};

struct choice_entry_ {
  Generic choice_handle;
  char *arg_str, *name, *help;
};

struct choice_ {
  OPT_CHOICE_CLBK_FN callback;
  char *name, *help;
  int num_choices;	/* a max of 10 supported for now -- Hood */
  struct choice_entry_ *choice;
};


struct string_ {
  OPT_STRING_CLBK_FN callback;
  char *name, *help;
  int maxlen;
  int display_len;
  char *regex;
};

typedef struct option_ {
  option_type t;
  char arg_char;
  Generic init_value;
  Boolean in_dialog;
  Generic f_c; //  struct flag_ *,  * struct choice_ *,	or * struct string_ *
} Option;


// class OptionsIterator; // forward declaration

class Options {
  private:
    struct OptionsS *hidden;

  public:
    Options(char *options_title);
   ~Options();

    void Add(Option *opt);
    void Usage(char *exec_name);
  
    friend class OptionsIterator;
};

class OptionsIterator {  
  private:
    struct OptionsIteratorS *hidden;

  public:
    OptionsIterator(Options *opt);
   ~OptionsIterator();
    Option *Current();
    void operator ++();
    void Reset();
};


EXTERN(Boolean, opt_parse_with_dialog, 
       (Options *opts, void *handle, int argc, char **argv));

EXTERN(int, opt_parse_argv,	
       (Options *opts, void *handle, int argc, char **argv));

EXTERN(Boolean,	opt_dialog_ok,	
       (Options *opts, char *exec_name, int *p_argc, char ***p_argv));

EXTERN(char, *opt_filter_string,
       (Options *opts, char arg_char, int *argc_p, char ***argv_p));

EXTERN(int, opt_get_choice,
       (Options *opts, char arg_char, int argc, char **argv));

# endif
