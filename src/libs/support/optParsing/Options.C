/* $Id: Options.C,v 1.5 1997/03/27 20:51:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>

#include <libs/support/optParsing/Options.h>
#include <libs/support/lists/SinglyLinkedList.h>

// Options.C
//
// data structure for managing options processing with opt_parse_argv
// JMC 2/93

Options::Options(char *options_title)
{
  hidden = new OptionsS(options_title);
}


Options::~Options()
{
  delete hidden;
}

class OptionsListEntry : public SinglyLinkedListEntry {
public:
  Option *opt;
  OptionsListEntry(Option *o) : opt(o) {};
};

void Options::Add(Option *opt)
{
  OptionsListEntry *e = new OptionsListEntry(opt);
  hidden->olist.Append(e);
}

void Options::Usage(char *exec_name)
{
  fprintf(stderr, "usage: %s\n", exec_name);
  OptionsIterator oi(this);
  for (Option *opt; opt = oi.Current(); ++oi) {
    char *help_text;
    fprintf(stderr, "   -%c ", opt->arg_char);
    switch(opt->t) {
    case flag:
      help_text = ((struct flag_ *) opt->f_c)->help;
      fprintf(stderr, "%s\n", help_text);
      break;
    case choice: {
      struct choice_ *c = (struct choice_ *) opt->f_c;
      help_text = c->help;
      fprintf(stderr, "%s", c->choice[0].arg_str);
      for (int i=1; i < c->num_choices; i++)
	fprintf(stderr, "|%s", c->choice[i].arg_str);
      fprintf(stderr, " %s\n", help_text);
      break;
    }
    case string:
      help_text = ((struct string_ *) opt->f_c)->help;
      fprintf(stderr, "<%s>\n", help_text);
      break;
    }
  }
}

OptionsIterator::OptionsIterator(Options *opts)
{
  hidden = new OptionsIteratorS(&opts->hidden->olist);
}

OptionsIterator::~OptionsIterator()
{
  delete hidden;
}

void OptionsIterator::operator ++()
{
  ++hidden->it;
}

void OptionsIterator::Reset()
{
  hidden->it.Reset();
}


Option *OptionsIterator::Current()
{
  OptionsListEntry *e = (OptionsListEntry *) hidden->it.Current();
  return (e ? e->opt : 0); // handle end of list case properly -- JMC 2/93
}


