# include<stdio.h>
# include<stdlib.h>
# include<math.h>

int *bimodal_counter,*gshare_counter,ghr=0,pcbits,pcbits_g,pcbits_b,ghrbits,hybrid_counter_bits,g_mask=0x00000001,*hybrid_counter; 
char *mode;

int *allocate1Darray(int);
void initialise1Darray(int *,int);
void initialise_hybrid_counter(int *,int);
int calculate_index(int address, int pcbits);
int calculate_index_gshare(int address, int pcbits, int ghrbits);
char prediction(int index,int *counter);
void update_bhr(char);
void update_hybrid_counter(int *hybrid_counter,int index_hybrid,char result,char predict_bimodal,char predict_gshare);

int *allocate1Darray(int index)
{
	int *matrixarray=(int*)malloc(sizeof(int)*index);
	return matrixarray;
}

void initialise1Darray(int *matrixarray,int number_of_predictors)
{
	int i;
	for(i=0;i<number_of_predictors;i++)
	{
		matrixarray[i]=2;
	}
}
void initialise_hybrid_counter(int *matrixarray,int number_of_predictors)
{
	int i;
	for(i=0;i<number_of_predictors;i++)
	{
		matrixarray[i]=1;
	}
}
int calculate_index(int address, int pcbits)
{
	int mask=0x00000001,index;
	address=address>>2;
	mask=(mask<<pcbits)-1;
	index=mask&address;
	return index;
}
int calculate_index_gshare(int address, int pcbits, int ghrbits)
{
	int mask=0x00000001,mask1=0x00000001,index,index1,temp;
	//printf("given address : %x\n",address);
	address=address>>2;
	mask=(mask<<pcbits)-1;
	index=address&mask;                   //extracting m bits from program counter
	index1=address&mask;
	index1=index1>>(pcbits-ghrbits);        //extracting n bits from m bits
	index1=index1^ghr;                      //XOR'in n bits with n bits ghr counter
	//printf("after XOR with ghr index : %d\n",index1);
	index1=index1<<(pcbits-ghrbits);
	mask1=(mask1<<(pcbits-ghrbits))-1;
	temp=index&mask1;
	index=index1|temp;
	//printf("final index concatenated value: %d\n", index);
	return index;
}
	
char prediction(int index,int *counter)
{
	char predict;
	if(counter[index]==0 || counter[index]==1)
	predict='n';
	if(counter[index]==2 || counter[index]==3)
	predict='t';
	return predict;
}
void main(int argc,char *argv[])
{
	char result,predict,predict_bimodal,predict_gshare;
	int address,i,index,index_bimodal,index_gshare,index_hybrid,number_of_predictors,number_of_bimodal_predictors,number_of_gshare_predictors,number_of_hybrid_predictors,misprediction=0,k;
	FILE *fpr;
	mode=argv[1];
	if(mode[0]=='b')
	{
		fpr=fopen(argv[5],"r");
		pcbits=atoi(argv[2]);
		number_of_predictors=pow(2,pcbits);
		bimodal_counter=allocate1Darray(number_of_predictors);
		initialise1Darray(bimodal_counter,number_of_predictors);
		while(!feof(fpr))
		//for(k=1;k<=10000;k++)
		{
			fscanf(fpr," %x %c\n",&address,&result);
			index=calculate_index(address,pcbits);
			predict=prediction(index,bimodal_counter);
			if(predict!=result)
			misprediction++;
			if(result=='t' && bimodal_counter[index]!=3)
			bimodal_counter[index]++;
			if(result=='n' && bimodal_counter[index]!=0)
			bimodal_counter[index]--;
		}
		printf("%d\n",misprediction);
		for(i=0;i<number_of_predictors;i++)
		printf("%d %d\n",i,bimodal_counter[i]);
	}
	if(mode[0]=='g')
	{
		fpr=fopen(argv[6],"r");
		pcbits=atoi(argv[2]);
		number_of_predictors=pow(2,pcbits);
		ghrbits=atoi(argv[3]);
		gshare_counter=allocate1Darray(number_of_predictors);
		initialise1Darray(gshare_counter,number_of_predictors);
		g_mask=(g_mask<<ghrbits)-1;
		ghr=ghr&g_mask;
		while(!feof(fpr))
		//for(k=1;k<=50;k++)
		{
			fscanf(fpr," %x %c\n",&address,&result);
			index=calculate_index_gshare(address,pcbits,ghrbits);
			printf("%d. PC: %x %c\n",k,address,result);
			printf("GSHARE index: %d old value: %d ",index,gshare_counter[index]);
			predict=prediction(index,gshare_counter);
			if(predict!=result)
			misprediction++;
			//printf("%d\n\n",misprediction);
			if(result=='t' && gshare_counter[index]!=3)
				gshare_counter[index]++;
			if(result=='n' && gshare_counter[index]!=0)
				gshare_counter[index]--;
			update_bhr(result);
			printf("new value %d\n",gshare_counter[index]);
			printf("BHR updated: %d\n",ghr);
		}
		for(i=0;i<number_of_predictors;i++)
		printf("%d %d\n",i,gshare_counter[i]);
	}
	if(mode[0]=='h')
	{
		fpr=fopen(argv[8],"r");
		hybrid_counter_bits=atoi(argv[2]);
		number_of_hybrid_predictors=pow(2,hybrid_counter_bits);
		pcbits_g=atoi(argv[3]);
		number_of_gshare_predictors=pow(2,pcbits_g);
		ghrbits=atoi(argv[4]);
		pcbits_b=atoi(argv[5]);
		number_of_bimodal_predictors=pow(2,pcbits_b);
		hybrid_counter=allocate1Darray(number_of_hybrid_predictors);
		initialise_hybrid_counter(hybrid_counter,number_of_hybrid_predictors);
		gshare_counter=allocate1Darray(number_of_gshare_predictors);
		initialise1Darray(gshare_counter,number_of_gshare_predictors);
		g_mask=(g_mask<<ghrbits)-1;
		ghr=ghr&g_mask;
		bimodal_counter=allocate1Darray(number_of_bimodal_predictors);
		initialise1Darray(bimodal_counter,number_of_bimodal_predictors);
		while(!feof(fpr))
		//for(k=1;k<=50;k++)
		{
			fscanf(fpr," %x %c\n",&address,&result);
			index_gshare=calculate_index_gshare(address,pcbits_g,ghrbits);
			index_bimodal=calculate_index(address,pcbits_b);
			predict_gshare=prediction(index_gshare,gshare_counter);  //step 1
			predict_bimodal=prediction(index_bimodal,bimodal_counter);
			index_hybrid=calculate_index(address,hybrid_counter_bits); //step 2
			if(hybrid_counter[index_hybrid]>=2)  //step 3 & 4
			{
				predict=predict_gshare;
				if(result=='t' && gshare_counter[index_gshare]!=3)
					gshare_counter[index_gshare]++;
				if(result=='n' && gshare_counter[index_gshare]!=0)
					gshare_counter[index_gshare]--;
			}
			else
			{
				predict=predict_bimodal;
				if(result=='t' && bimodal_counter[index_bimodal]!=3)
					bimodal_counter[index_bimodal]++;
				if(result=='n' && bimodal_counter[index_bimodal]!=0)
					bimodal_counter[index_bimodal]--;
			}
			update_bhr(result); //step 5
			if(predict!=result)
			misprediction++;
			update_hybrid_counter(hybrid_counter,index_hybrid,result,predict_bimodal,predict_gshare); //step 6
		}
		printf("misprediction: %d\n",misprediction);
		for(i=0;i<number_of_hybrid_predictors;i++)
		printf("%d %d\n",i,hybrid_counter[i]);
	}
}
void update_bhr(char result)
{
	int outcome;
	if(result=='t')
	{
		outcome=1;
		outcome=outcome<<(ghrbits-1);
		ghr=(ghr>>1);
		ghr=ghr|outcome;
		ghr=ghr&g_mask;
	}
	if(result=='n')
	{
		outcome=0;
		outcome=outcome<<(ghrbits-1);
		ghr=(ghr>>1);
		ghr=ghr|outcome;
		ghr=ghr&g_mask;
	}
}
void update_hybrid_counter(int *matrixarray,int index,char result,char predict_bimodal,char predict_gshare)
{
	if(result==predict_gshare && result!=predict_bimodal && matrixarray[index]!=3)
	matrixarray[index]++;
	if(result==predict_bimodal && result!=predict_gshare && matrixarray[index]!=0)
	matrixarray[index]--;
}		
	