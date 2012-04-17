#!/bin/sh



# TIMEFORMAT
#The value of this parameter is used as a format string specifying how the timing information for pipelines prefixed with the time reserved word should
# be displayed.  The % character introduces an escape sequence that is expanded to a time value or other information.  The escape  sequences  and  their
# meanings are as follows; the braces denote optional portions.
# %%        A literal %.
# %[p][l]R  The elapsed time in seconds.
# %[p][l]U  The number of CPU seconds spent in user mode.
# %[p][l]S  The number of CPU seconds spent in system mode.
# %P        The CPU percentage, computed as (%U + %S) / %R.
# The  optional  p  is a digit specifying the precision, the number of fractional digits after a decimal point.  A value of 0 causes no decimal point or
# fraction to be output.  At most three places after the decimal point may be specified; values of p greater than 3 are changed to 3. 
# If p is not specified, the value 3 is used.
# The  optional  l  specifies  a  longer  format,  including  minutes,  of the form MMmSS.FFs.  The value of p determines whether or not the fraction is
# included.
# If this variable is not set, bash acts as if it had the value $'\nreal\t%3lR\nuser\t%3lU\nsys%3lS'.  If the value is null, no  timing  information  is
# displayed.  A trailing newline is added when the format string is displayed.
#
#																										   

#TIMEFORMAT="Total time in secs = %lR, CPU in USER MODE  %lU, CPU in SYSTEM MODE %lS, CPU PERCENTAGE %P "

#`/usr/local/condor/bin/condor_submit host_job.cmd`
 
TIMEFORMAT="Total time in secs = %R, CPU in USER MODE  %U, CPU in SYSTEM MODE %S, CPU PERCENTAGE %P "

time {

while :
do

#	 `condor_q | wc -l > /tmp/cvh_tmp999`
#	 QUEUE=`cat /tmp/cvh_tmp999`
#	 QUEUE=`expr $QUEUE - 6 `
#	 `rm -fr /tmp/cvh_tmp999`
#	 echo "$QUEUE"

#	QUEUE=`condor_q | wc -l `

        QUEUE=`condor_q | grep jobs | cut   --delimiter=";"  -f 1 | cut --delimiter=" " -f 1`
	if test $QUEUE -eq 0
	then 
		break	
	fi
	 


done



}
