BEGIN {
	print "/* This include file is generated by a bogus awk script... */"
	print "/* Try regenerating it with 'awk -f astnode.awk nodedef.def' */"
	print ""
	print "#ifndef astnode_h"
	print "#define astnode_h"
	print ""
        count = 0
      }
/struct/{
	next
	}
/"/{
        name = substr($1,2,length($1)-2)
	prefix = name
        printf ("#define GEN_%s %d\n", prefix, count)
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
        printf ("#define GEN_LAST %d\n", count)
	print ""
	print "#endif"
      }
