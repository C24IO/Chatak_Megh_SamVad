/*Program for the finding out of the runtime cpu statistics .*/


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>




struct cpu_info {
	unsigned long long user;
	unsigned long long system;
	unsigned long long idle;
	unsigned long long iowait;
} new_cpu, old_cpu;

FILE *cpufp;			/* /proc/stat */
char buffer[256];		/* Temporary buffer for parsing */


unsigned int n_partitions;	/* Number of partitions */
unsigned int ncpu;		/* Number of processors */
unsigned int kernel;		/* Kernel: 4 (2.4, /proc/partitions)*/



void handle_error(const char *string, int error)
{
	if (error) {
		fputs("iostat: ", stderr);
		if (errno)
			perror(string);
		else
			fprintf(stderr, "%s\n", string);
		exit(EXIT_FAILURE);
	}
}

void get_number_of_cpus()
{
	FILE *ncpufp = fopen("/proc/cpuinfo", "r");

	handle_error("Can't open /proc/cpuinfo", !ncpufp);
	
	while (fgets(buffer, sizeof(buffer), ncpufp)) {
		
		if (!strncmp(buffer, "processor\t:", 11))
			
			ncpu++;
	}
	
	fclose(ncpufp);
	
	handle_error("Error parsing /proc/cpuinfo", !ncpu);
}


struct cpu_info cpu_stats()
{
	double total;
	struct cpu_info cpu;

	setlinebuf(stdout);
	get_number_of_cpus();
	
	memset((void *)&old_cpu,0,sizeof(old_cpu));
	memset((void *)&new_cpu,0,sizeof(new_cpu));

	cpufp = fopen("/proc/stat", "r");
	handle_error("Can't open /proc/stat", !cpufp);

	rewind(cpufp);
	while (fgets(buffer, sizeof(buffer), cpufp)) {
		if (!strncmp(buffer, "cpu ", 4)) {
			int items;
			unsigned long long nice, irq, softirq;

			items = sscanf(buffer,
				     "cpu %llu %llu %llu %llu %llu %llu %llu",
				       &new_cpu.user, &nice,
				       &new_cpu.system,
				       &new_cpu.idle,
				       &new_cpu.iowait,
				       &irq, &softirq);

			new_cpu.user += nice;
			if (items == 4)
				new_cpu.iowait = 0;
			if (items == 7)
				new_cpu.system += irq + softirq;

		}
	}

/*	old_cpu = new_cpu;

	sleep(0);

	rewind(cpufp);
	while (fgets(buffer, sizeof(buffer), cpufp)) {
		if (!strncmp(buffer, "cpu ", 4)) {
			int items;
			unsigned long long nice, irq, softirq;

			items = sscanf(buffer,
				     "cpu %llu %llu %llu %llu %llu %llu %llu",
				       &new_cpu.user, &nice,
				       &new_cpu.system,
				       &new_cpu.idle,
				       &new_cpu.iowait,
				       &irq, &softirq);

			new_cpu.user += nice;
			if (items == 4)
				new_cpu.iowait = 0;
			if (items == 7)
				new_cpu.system += irq + softirq;

		}
	}

*/	



	cpu.user = new_cpu.user - old_cpu.user ;
        cpu.system = new_cpu.system - old_cpu.system ;
	cpu.idle = new_cpu.idle - old_cpu.idle ;
	cpu.iowait = new_cpu.iowait - old_cpu.iowait ;

	total = (cpu.user + cpu.system + cpu.idle + cpu.iowait) / 100.0;


	/*printf("%3.3f %3.3f ", cpu.user /total , cpu.system /total);
	printf("%3.3f ", cpu.iowait /total);
	printf("%3.3f \n ", cpu.idle/total );*/

	
	printf("%f %f ", cpu.user /total , cpu.system /total);
	printf("%f ", cpu.iowait /total);
	printf("%f \n ", cpu.idle/total );
	return(cpu);		

	
}

int main(void){

	cpu_stats();
}
	
