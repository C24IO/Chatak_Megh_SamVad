/* This program is CPU intensive not memory and IO intensive, doing the bit wise operators in a loop */







void disp_binary (int i){

	register int t;

	for( t = 128 ; t > 0 ; t = (t/2) )

		if( i & t ) printf("1");
		else printf("0");

	printf("\n");
}

int main(void){


	int i = 1 , t;

	while(1){

	for( t=0 ; t<8 ; t++){

		disp_binary(i);

		i = i << 1;
	}


	printf("\n");

	for( t=0 ; t<8 ; t++){

		i = i >> 1;
		disp_binary(i);
	}

	
	}




}

	
