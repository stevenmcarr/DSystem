BEGIN {
        printf ("#ifndef RN_LINT_LIBRARY\n")
	print ""
        count = 1
      }
/struct/{
	next
	}
/"/{
        name = substr($1,2,length($1)-2)
	prefix = name
        printf ("\"%s\",%d,", prefix, $5)
        if (count % 5 == 0) printf("\n")
        count++
	next
      }
/\/\*/{
	next
      }
/{/{
       next
      }
/}/{
       next
      }
END   {
      printf ("\n")
      printf ("#endif\n")
      }
