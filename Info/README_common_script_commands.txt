Mike Chirico (mchirico@users.sourceforge.net) (mchirico@comcast.net)
Copyright (c) 2004 (GPU Free Documentation License) 
Last Updated: Thu Jul 22 15:05:46 EDT 2004

The latest version of this document can be found at:
http://prdownloads.sourceforge.net/souptonuts/README_common_script_commands.txt?download

Programs and scripts that go along with this document:
http://prdownloads.sourceforge.net/souptonuts/common_script_commands.tar.gz?download


Automating FTP, generating email alerts, sending GPG encrypted email from
the command prompt with the appropriate "pgp-encrypted" mime types, and running MySQL
from the command without exposing passwords, are all detailed below.


SCRIPT 1:

       Automating FTP.

       The passwords for "ftp"  can be stored in the  "~/.netrc" file. Here's
       an example file that specifies 3 different logins. A default login using
       "anonymous" and password user@site. A different password for upload.sourceforge.net,
       and the login "bobby" and password "b0bb13" only for computer 192.168.1.35.
        
          $  cat ~/.netrc

               default login anonymous password user@site
               machine upload.sourceforge.net login anonymous password m@temp.com
               machine 192.168.1.35 login bobby password b0bb13

       Once created this file should be protected as follows:

          $  chmod 0400 ~/.netrc

       Next, the following script will create two directories  "/faq/unix-faq" under home, without error,
       if they do not exist. This is the advantage of using the "mkdir" with the "-p" option, which can
       create several layers of subdirectories, or if all directories exist does not return with an error.

          #!/bin/bash                                            
          #                                                      
          #  Sample ftp automated script to download             
          #  file to $dwnld                                      
          #                                                      
          dwnld=~/faq/unix-faq                                   
          mkdir -p $dwnld                                        
          cd $dwnld                                              
          ftp << FTPSTRING                                       
          prompt off                                             
          open rtfm.mit.edu                                      
          cd /pub/usenet-by-group/news.answers/unix-faq/faq      
          mget contents                                          
          mget diff                                              
          mget part*                                             
          bye                                                    
          FTPSTRING                                              

       Note: quotes are not used in the initial assignment of the  variable dwnld. 
             Without quotes, it refers to the user's home "/home/chirico/faq/unix-faq"
             for the user chirico.

                 $ dwnld=~/faq/unix-faq    # This is good. Refers  /home/chirico/faq/unix-faq

                 $ dwnld="~/faq/unix-faq"  # No good. Refers to a directory '~' at current location

      

SCRIPT 2:

       Using find.

       The following will find all files > 20k in the current directory.

            $ find . -size +20K

       The following will find all files, except  "/CVS" and the files under it.
       Plus, any files ending with a "~" will be ignored; but "here~is" will be
       picked up in this find.  

            $ find . \( -iregex '.*/CVS'  -o -iregex '.*~' \)  -prune -o -print

       The above command can be used for creating a "tar.gz" for all the files under
       the "test" directory. The "-H ustar" is for the gnu/posix version of tar.

            $ find ./test \( -iregex '.*/CVS'  -o -iregex '.*~' \)  -prune -o -print | cpio -o -H ustar |gzip > data.tar.gz

       But wait, you now may want to append additional files, for instance what is under "./test2",  to "data.tar.gz"

            $ gunzip data.tar.gz
            $ find ./test2 \( -iregex '.*/CVS'  -o -iregex '.*~' \)  -prune -o -print | cpio -o -H ustar --append -F data.tar
            $ gzip < data.tar > data.tar.gz

       That last command "$ gzip < data.tar > data.tar.gz"  could have been done with just "$ gzip data.tar".  But, the shorter
       command deletes data.tar, which could be useful if more appends are needed.


       Find all '*.c' files with the 'current->signal->tty' in them, ignoring errors. 
  
            $ find . -iname '*.c' -exec grep -H 'current->signal->tty' {} \; 2>/dev/null

               ./arch/i386/mach-voyager/voyager_thread.c:      current->signal->tty = NULL;
               ./arch/sparc64/solaris/misc.c:                  current->signal->tty = NULL;
               ./arch/ia64/kernel/unaligned.c:                         tty_write_message(current->signal->tty, buf);
               ./drivers/net/slip.c:           if (sl->tty != current->signal->tty && sl->pid != current->pid) {
               ...

       Note above, the file name is given on the left.  Stuff like this can be great for finding changes in the kernel. By 
       the way, "current->tty" was recently replaced with "current->signal->tty" in 2.6+ versions of the Linux kernel.


       Suppose you want 3 lines before "-B" the  grepped text and 2 lines after "-A". Then, use the "-A" for after and "-B" 
       for before as follows.

             $ find . -iname '*.c' -exec grep -H -A 2 -B 3 'current->signal->tty' {} \; 2>/dev/null

                ./kernel/exit.c-        exit_mm(current);                
                ./kernel/exit.c-                                         
                ./kernel/exit.c-        set_special_pids(1, 1);          
                ./kernel/exit.c:        current->signal->tty = NULL;     
                ./kernel/exit.c-                                         
                ./kernel/exit.c-        /* Block and flush all signals */


       INODES.

         It's possible to create a file or directory with any character except '/' or the '\0' character. 


             $ touch  'this~\nis one big file here' 
             
         To get the inode of the file

             $ ls -il th*

                 75489 -rw-rw-r-- 1 chirico  chirico 0 Jul 20 15:28 this~\nis one big file here

         Find can look for a particular inode as follows:

             $ find . -inum 75489

         To easily remove this file by its inode

             $ find . -inum 75489 -exec rm {} \;


      FIND and REPLACE

         This can be difficult. Should replacements be made in executable files? And not all replacements should be made.. consider 
         files under the the ./CVS directory.  For an example program, see the downloads.

      OTHER USES

       Find all files, starting at the root, accessed < n*24 hours ago ignoring errors.
       Here n=-1.  For > 24 hours set n=+1, and "exactly" 24 hours ago n=1.

         $ find / -atime -1 2>/dev/null

       

SCRIPT 3:

      Email alerts

      Assume the following file "data"

            $ cat data
               1 one   
               2 two   
               3 three 
               4 four  

      "awk" can zero in on line and column.

            $ awk '/3/ { print $2 }' data
               three

      And, system commands can be run as well. Following is an example using
      "wc" word count.

            $ awk '/3/ { print $2| "wc" }' data
              1       1       6
        
      Or, to email data with the subject  " Useless Topic Matching 3"  to 
      mchirico@comcast.net. \x27 is equal to a single quote, "man ascii".

            $ awk '/3/ { print $2| "mail -s \x27 Useless Topic Matching 3 \x27 mchirico@comcast.net "}' data

      This following command emails io stats, with "uname -a" giving the full computer name. The date includes the
      month, day, year and time, down to the nano-second. 

            $ (uname -a;date "+%m.%d.%y %H:%M:%S.%N";iostat) | awk '{print $0|"mail -s \x27 iostats \x27 mchirico@comcast.net "}' 


      With awk sometimes it's necessary to pass along a  variable.  Consider a script that will kill every 
      process for a user. Here the first argument passed to the shell script is being passed along
      to awk.

         #!/bin/bash                                                      
         #
         #  This program will kill all processes from a         
         #  user.  The user name is read from the command line. 
         #                                                      
         #  Usage: kill9user <user>                             
         #                                                      
         kill -9 `ps aux|awk -v var=$1 '$1==var { print $2 }'`  



SCRIPT 4:

      Sending GPG Encrypted Email from the Command Line.

      GPG the GNU version of PGP is a way of encrypting data easily
      using a public key.  This is ideal. No transferring or communicating
      a password between parties.

      For instance, the following command will encrypt the data in 
      file.txt.

         $ openssl des3 -salt -in file.txt -out file.des3 -k secretPassword

      However, when the recepient gets this file, that user must know "secretPassword"
      to decrypt.  Or, if this is done in a batch job, the password may be exposed.

         $ openssl des3 -d -salt -in file.des3 -out file.txt -k secretPassword

      GPG avoids this problem.  There is no sharing of passwords. For a more detailed
      description of GPG see the references below.

      It's not enough to encrypt the email message and embed it into the body of the message.
      The mime type should be set as "pgp-encrypted", or else the user is 
      forced to copy and past the message, or develop a script to extract 
      the text. This is an example that embeds the  message and sets 
      the mime type correctly with a boundary.  Users that have gpg or pgp 
      configured emails  will automatically be prompted for a password.

      Ideally the script should work as follows

         $  ls -l|./sndmailBash fruser@isp.net touser@isp.net "Example ls -l cmd" 0xA11C1499               

      The contents of the "ls -l" command is encrypted into the body of the message, it's
      from the user "fruser@isp.net" and to the user "touser@isp.net". The subject of the
      message is "Example ls -l cmd" and it's encrypted with the pgp key 0xA11C1499.

      The "sndmailBash" program will set the "Content-Type" to be pgp-encrypted, by piping
      this to the "sendmail -t" cmd. The "-t" option is to parse prefixes like "From: ", 
      "To: ", "Subject: ", etc.  The boundary is static, but should be unique.

      Here is the "sndmailBash" script that will do this:

       #!/bin/bash                                                                                               
       # Created: Mike Chirico mchirico@users.sourceforge.net or mchirico@comcast.net                            
       # Copyright GPL 2004                                                                                      
       # Last Updated: Wed Jul 21 14:14:10 EDT 2004                                                              
       # Download: http://prdownloads.sourceforge.net/souptonuts/common_script_commands.tar.gz?download          
       #                                                                                                         
       # Sample usage:                                                                                           
       #  $  ./sndmailBash <from> <to> "<subject>"  <gpgkey>  < file
       #         or
       #  $  some command|./sndmailBash <from> <to> "<subject>"  <gpgkey>  
       #
       # Specific example:                                                    
       #                                                                                                         
       #  $  ./sndmailBash mchirico@comcast.net mchirico@comcast.net "Subject here" 0xA11C1499 < data            
       #               or                                                                                        
       #  $  ls -l|./sndmailBash mchirico@comcast.net mchirico@comcast.net "ls listing" 0xA11C1499               
       #                                                                                                         
       #     The above command will send an email from "mchirico@comcast.net" to "mchirico@comcast.net"          
       #     with the subject "ls listing" encrypted with the pgp key "0xA11C1499".                              
       #                                                                                                         
       #     The command below will list the gpg keys.                                                           
       #                                                                                                         
       #      $ gpg --list-keys                                                                                  
       #            /home/chirico/.gnupg/pubring.gpg                                                             
       #            --------------------------------                                                             
       #            pub  1024D/A11C1499 2004-07-15 Mike Chirico  <mchirico@comcast.net>                          
       #                        ^-------                                                                         
       #                               |___ add "0x" for "0xA11C1499"                                            
       #                                                                                                         
       #      Obviously this is my key, so add the correct recipient key.                                        
       #                                                                                                         
       From=${1}                                                                       
       To=${2}                                                                         
       Subject=${3}                                                                    
                                                                                       
       #Content=`cat | gpg  -r ${4}  --encrypt --armor `                               
       Content=$(gpg  -r ${4}  --encrypt --armor )                                     
                                                                                       
                                                                                       
       /usr/sbin/sendmail -t <<EOF                                                     
       From: ${From}                                                                   
       To: ${To}                                                                       
       Subject: ${Subject}                                                             
       Mime-Version: 1.0                                                               
       Content-Type: multipart/encrypted; protocol="application/pgp-encrypted";        
               boundary="B835649000072104Jul07"                                        
       Content-Disposition: inline                                                     
       User-Agent: Mutt/1.4.1i                                                         
                                                                                       
                                                                                       
       --B835649000072104Jul07                                                         
       Content-Type: application/pgp-encrypted                                         
       Content-Disposition: attachment                                                 
                                                                                       
       Version: 1                                                                      
                                                                                       
       --B835649000072104Jul07                                                         
       Content-Type: application/octet-stream                                          
       Content-Disposition: inline; filename="msg.asc"                                 
                                                                                       
       ${Content}                                                                      
                                                                                       
                                                                                       
       --B835649000072104Jul07--                                                       
                                                                                       
       EOF                                                                             
                                                                                       

     If you want more detail on gpg, reference (TIP 86) at
      http://prdownloads.sourceforge.net/souptonuts/How_to_Linux_and_Open_Source.txt?download



SCRIPT 5:

       Working with MySQL on the Command Line.

       The following command will query the "exams" table in the "test" database.

             $ mysql --skip-column-names -s -e "select * from exams" test

                1       Bob     1       75                
                2       Bob     2       77
                3       Bob     3       78
                4       Bob     4       80
                5       Sue     1       90
                6       Sue     2       97
                7       Sue     3       98
                8       Sue     4       99

       To prevent password and username prompting, create a ".my.cnf" in
       the user's home directory  "/home/chirico/.my.cnf" and enter
       the username and password, similar to the following.
      
             [client]         
             user=user1
             password=p@ssw0rd

       The above method is the recommended way, since other methods
       may show the password when anyone does a "ps -aux" command.




REFERENCES:

(Programs for this document)
   http://prdownloads.sourceforge.net/souptonuts/common_script_commands.tar.gz?download

(awk and shell)
   http://www-106.ibm.com/developerworks/library/l-awk3.html
   http://sparky.rice.edu/~hartigan/awk.html
   http://www.shelldorado.com/
   http://www.intuitive.com/wicked/index.shtml

(GPG)
   http://www.gnupg.org/documentation/faqs.html
   http://codesorcery.net/mutt/mutt-gnupg-howto
   http://www.gnupg.org/(en)/download/index.html


OTHER TIPS:

For tips on MySQL reference:
http://prdownloads.sourceforge.net/souptonuts/README_mysql.txt?download

For tips on upgrading RedHat 9 or 8.0 to 2.6.x src kernel
with Fedora Updates, reference:
http://prdownloads.sourceforge.net/souptonuts/README_26.txt?download

For tips on Comcast Email with Home Linux Box
http://prdownloads.sourceforge.net/souptonuts/README_COMCAST_EMAIL.txt?download




































bind '"\C-t": "mutt\n"'
 
