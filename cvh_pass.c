
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

extern int CkptInProgress;

main( argc, argv)
int		argc;
char	*argv[];
{
	int a[9];
	int i,j,k,l,m,n,o,c;
	char b[10]="        ";
	clock_t start, end;

	printf("\n\nHi this is the password cracking program running......\n\n");

	printf("\n\n\n\n");
	start = clock();

	strcpy(b,argv[1]);

	for(i=33;i!=256;i++)
	{ 
		a[0]=i;
		printf("1");
		for(j=33;j!=256;j++)
		{
			printf("2");
			a[1]=j;
		for(k=33;k!=256;k++)
		{
			printf("3");
			a[2]=k;
			for(l=33;l!=256;l++)
			{
				printf("4");
				a[3]=l;
			for(m=33;m!=256;m++)
			{
				printf("5");
				a[4]=m;
				for(n=33;n!=256;n++)
				{
					
					
					a[5]=n;				
					for(o=33;o!=256;o++)
					{
						
						a[6]=o;
						for(c=33;c!=256;c++)
						{
						
							
							a[7]=c;

         // printf("%c%c%c%c%c%c%c%c ",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);

		if((a[0]==b[0])&&(a[1]==b[1])&&(a[2]==b[2])&&(a[3]==b[3])&&(a[4]==b[4])&&(a[5]==b[5])&&(a[6]==b[6])&&(a[7]==b[7]))
		{
		
			printf("THE PASSWORD IS CRACKED\n\n");
			printf("%d %d %d %d %d %d %d %d\n\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
			goto end;
		}
						}
					}

				}
			}
			}
		}
		
		}
	}
end:;
	end = clock();


/*			          printf ( "The Crack took %f seconds\n", difftime ( end, start ) );*/

	  printf ( "The loop took %f seconds\n",(double)( end - start ) / (double)CLOCKS_PER_SEC );

	  
}
