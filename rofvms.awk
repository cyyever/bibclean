# rofvms.awk -*-awk-*-
# Filter to convert nroff -man output to VMS .hlp file format according
# to the rules:
#
#	13 or more consecutive blank lines are reduced to 1
#	3--12 consecutive blank lines are dropped
#	2 consecutive blank lines are reduced to 1
#	All others output verbatim.
#
# The peculiar number 13 handles the case where a paragraph break
# coincides with a page break.
#
# In addition, whenever a line in non-blank in column 1, and then
# previous line was blank, we insert a blank line; this provides
# vertical space before a section heading.
#
# The output of nroff -man on different UNIX systems is regrettably
# quite variable in appearance; this file is likely to need
# modifications on other than Sun OS.
#
# Too bad nroff doesn't have an option to suppress titling!
#
# The NAME section head becomes 1 BIBCLEAN, and others become
# 2 XXX followed by XXX.
# [06-Nov-1992]

# Match and delete page headers: xxx(nnn) .... xxx(nnn)
/^[A-Za-z][-_A-Za-z0-9]*\([0-9A-Za-z]+\).*[A-Za-z][-_A-Za-z0-9]*\([0-9A-Za-z]+\)$/  {next;}

# Match and delete page footers: Sun Release ...nnn
# These vary from system to system, so extra patterns may be needed here
/^Sun Release.*[0-9]+$/ {next;}	# Sun OS x.x
/^Printed.*[0-9]+$/ {next;}	# BSD 4.3
/^Page [0-9].*$/ {next;}	# Silicon Graphics
/^Version.*Last change:/ {next;}# bibclean.txt on SunOS 4.1.1


# Match all lines and do blank line processing
{
    if (NF == 0)	# blank line
	nb++;
    else		# non blank line
    {
	if ((nb == 1) || (nb == 2) || (nb >= 13))
	    printf("\n");
	else if ((nb > 0) && (substr($0,1,1) != " ") && (nf > 0))
	    printf("\n");
	if ($0 == "NAME")	# level 1 header
	    $0 = "1 BIBCLEAN";
	else if (substr($0,1,1) != " ") # level 2 header
	{
	    header = $0;
	    gsub(/ /,"-",header);
	    $0 = "2 " header "\n " $0;
	}
	printf("%s\n",$0);
	nb = 0;
	nf++;
    }
}
