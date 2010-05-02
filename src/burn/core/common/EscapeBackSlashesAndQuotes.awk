{
    # change single backslash to double backslash
    gsub(/\\/,"\\\\",$0); 

    # backslash all quotes
    gsub("\"","\\\"",$0);
	
    # output string and enclose it in quotes and \r\n, don't print first line
    if (FNR > 1)
        print "                            L\"\\r\\n" $0 "\"";
}
