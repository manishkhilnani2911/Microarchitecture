#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<assert.h>
struct cache
{
unsigned long cache_size;
int block_size,assoc,read_misses,write_misses,reads,writes;
int number_blocks,number_sets,block_offset_bits,index_bits,tag_bits,tag,index,index_mask;
int **tagmatrix,**lru_counter,**validbit;
char **dirtybit;
};
struct cache L1,L2;
struct victim_cache
{
unsigned long cache_size;
int block_size,number_blocks,number_sets,block_offset_bits,index_bits,tag_bits,tag,index,index_mask;
int *lru_counter,*tagmatrix,*validbit;
char *dirtybit;
};
struct victim_cache victim;
int  *allocate_victim_matrix();
char *allocate_victim_dirty_matrix();
void initialise_victim_matrix(int *);
void initialise_victim_dirty_matrix(char *);
void initialise_victim_lru_matrix(int *);
void update_lru_counter_victim(int);
void search_victim();
void write_victim(int,int);
void write_victim_when_victim_miss(int,int);
void decode_add_victim();
int writeback_address,flag;
int encode_writeback_address();
int index_mask=0x00000001,index_mask1=0x00000001;
int L1_reads=0,L1_writes=0,swaps=0,victim_writeback=0,cache_read_misses_L1=0,cache_write_misses_L1=0,L2_writes=0,L2_reads=0,write_backs_L2=0,cache_read_misses_L2=0,cache_write_misses_L2=0;
int **allocate_2Dmatrix(int , int);
char **allocate_dirty_matrix(int,int);
void initialise_2d_matrix(int **,int,int);
void initialise_lru_matrix(int **,int,int);
void initialise_dirty_matrix(char **,int,int);
void update_lru_counter_L1(int,int,int);
void update_lru_counter_L2(int,int,int);
void read_cache_L1();
void load_cache_address_L1(int,int,int,int);
void write_cache_L1();
void load_cache_wbwa_miss_L1(int,int,int,int);
void read_cache_L2(int);
void write_cache_L2(int);
int what_to_replace_L1(int,int);
int what_to_replace_L2(int,int);
void load_cache_address_L2(int,int,int,int);
void load_cache_wbwa_miss_L2(int,int,int,int);
void initialise_L1()
{
	L1.tagmatrix=allocate_2Dmatrix(L1.number_sets,L1.assoc);
	L1.lru_counter=allocate_2Dmatrix(L1.number_sets,L1.assoc);
	L1.validbit=allocate_2Dmatrix(L1.number_sets,L1.assoc);
	L1.dirtybit=allocate_dirty_matrix(L1.number_sets,L1.assoc);
	initialise_2d_matrix(L1.tagmatrix,L1.number_sets,L1.assoc);
	initialise_lru_matrix(L1.lru_counter,L1.number_sets,L1.assoc);
	initialise_2d_matrix(L1.validbit,L1.number_sets,L1.assoc);
	initialise_dirty_matrix(L1.dirtybit,L1.number_sets,L1.assoc);
}

void initialise_L2()
{
	L2.tagmatrix=allocate_2Dmatrix(L2.number_sets,L2.assoc);
	L2.lru_counter=allocate_2Dmatrix(L2.number_sets,L2.assoc);
	L2.validbit=allocate_2Dmatrix(L2.number_sets,L2.assoc);
	L2.dirtybit=allocate_dirty_matrix(L2.number_sets,L2.assoc);
	initialise_2d_matrix(L2.tagmatrix,L2.number_sets,L2.assoc);
	initialise_lru_matrix(L2.lru_counter,L2.number_sets,L2.assoc);
	initialise_2d_matrix(L2.validbit,L2.number_sets,L2.assoc);
	initialise_dirty_matrix(L2.dirtybit,L2.number_sets,L2.assoc);
}
void initialise_victim()
{
	victim.tagmatrix=allocate_victim_matrix();
	victim.lru_counter=allocate_victim_matrix();
	victim.validbit=allocate_victim_matrix();
	victim.dirtybit=allocate_victim_dirty_matrix();
	initialise_victim_matrix(victim.tagmatrix);
	initialise_victim_matrix(victim.validbit);
	initialise_victim_lru_matrix(victim.lru_counter);
	initialise_victim_dirty_matrix(victim.dirtybit);
}
int **allocate_2Dmatrix(int sets, int assoc)
{
	int **matrixarray=(int**)malloc(sizeof(int*)*sets);;
	int i;
	for(i=0;i<sets;i++)
		matrixarray[i]=(int*)malloc(sizeof(int)*assoc);
	return matrixarray;
}
//victim allocate and initialise functions start
int *allocate_victim_matrix()
{
	int *matrixarray=(int*)malloc(sizeof(int)*victim.number_blocks);
	return matrixarray;
}
char *allocate_victim_dirty_matrix()
{
	char *matrixarray=(char*)malloc(sizeof(char)*victim.number_blocks);
	return matrixarray;
}
void initialise_victim_matrix(int *matrixarray)
{
	int i;
	for(i=0;i<victim.number_blocks;i++)
	matrixarray[i]=0;
}
void initialise_victim_lru_matrix(int *matrixarray)
{
	int i;
	for(i=0;i<victim.number_blocks;i++)
	matrixarray[i]=(victim.number_blocks-i-1);
}
void initialise_victim_dirty_matrix(char *matrixarray)
{
	int i;
	for(i=0;i<victim.number_blocks;i++)
	matrixarray[i]=' ';
}
//victim initialise and allocate functions end
void initialise_2d_matrix(int **matrixarray,int sets,int assoc)
{
	int i,j;
	for(i=0;i<sets;i++)
	{
		for(j=0;j<assoc;j++)
		{
			matrixarray[i][j]=0;
		}
	}
}
void initialise_lru_matrix(int **matrixarray,int sets,int assoc)
{
	int i,j,number;
	for(i=0;i<sets;i++)
	{
		number=assoc-1;
		for(j=0;j<assoc;j++)
		{
			matrixarray[i][j]=number;
			number=number-1;
		}
	}
}
char **allocate_dirty_matrix(int sets,int assoc)
{
	int i;
	char **matrixarray=(char**)malloc(sizeof(char*)*sets);
	for(i=0;i<sets;i++)
	matrixarray[i]=(char*)malloc(sizeof(char)*assoc);
	return matrixarray;
}
void initialise_dirty_matrix(char **matrixarray,int sets,int assoc)
{
	int i,j;
	for(i=0;i<sets;i++)
	{
		for(j=0;j<assoc;j++)
		{
			matrixarray[i][j]=' ';
		}
	}
}
void calculate_L1_spec()
{
	L1.number_sets=L1.cache_size/(L1.block_size*L1.assoc);
	L1.block_offset_bits=(int)((log(L1.block_size))/log(2));
	L1.index_bits=(int)((log(L1.number_sets))/log(2));
	L1.tag_bits=32-L1.block_offset_bits-L1.index_bits;
	printf("%d %d %d",L1.block_offset_bits,L1.index_bits,L1.tag_bits);
	index_mask=(index_mask<<L1.index_bits)-1;
	//index_mask1=(index_mask1<<L1.tag_bits)-1;
}
void calculate_L2_spec()
{
	L2.number_sets=L2.cache_size/(L2.block_size*L2.assoc);
	L2.block_offset_bits=(int)((log(L2.block_size))/log(2));
	L2.index_bits=(int)((log(L2.number_sets))/log(2));
	L2.tag_bits=32-L2.block_offset_bits-L2.index_bits;
	printf("\n%d %d %d %d",L2.number_sets,L2.block_offset_bits,L2.index_bits,L2.tag_bits);
	index_mask1=(index_mask1<<L2.index_bits)-1;
}
void calculate_victim_spec()
{
	victim.number_blocks=victim.cache_size/victim.block_size;
	victim.block_offset_bits=(int)((log(victim.block_size))/log(2));
	victim.tag_bits=32-victim.block_offset_bits;
}
	
void decode_add_L1(int address)
{
	//400341a0
	//index_mask=(index_mask<<L1.index_bits)-1;
	//index_mask1=(index_mask1<<L1.tag_bits)-1;
	L1.tag=address;
	L1.tag=L1.tag>>(L1.block_offset_bits+L1.index_bits);
	//L1.tag=L1.tag&&index_mask1;
	L1.index=address;
	L1.index=L1.index>>L1.block_offset_bits;
	//L1.index=L1.index<<(L1.tag_bits+L1.block_offset_bits);
	//L1.index=L1.index>>(L1.tag_bits+L1.block_offset_bits);
	//index_mask=(index_mask<<L1.index_bits)-1;
	L1.index=L1.index&index_mask;
	//printf("%x %x\n",L1.tag,L1.index);
}
void decode_add_L2(int address)
{	//int address=0;
	//int index_mask1=0x00000001;
	L2.tag=address;
	L2.tag=L2.tag>>(L2.index_bits+L2.block_offset_bits);
	L2.index=address;
	L2.index=L2.index>>L2.block_offset_bits;
	L2.index=L2.index&index_mask1;
}
void decode_add_victim(int address)
{
	victim.tag=address;
	victim.tag=victim.tag>>victim.block_offset_bits;
}
void main(int argc,char* argv[])
{
	char command;
	int i,j,k,z,count,address;
	FILE *fpr;
	if(argc>1)
	{
		L1.block_size=atoi(argv[1]);
		L2.block_size=atoi(argv[1]);
		victim.block_size=atoi(argv[1]);
		L1.cache_size=atoi(argv[2]);
		L1.assoc=atoi(argv[3]);
		victim.cache_size=atoi(argv[4]);
		L2.cache_size=atoi(argv[5]);
		L2.assoc=atoi(argv[6]);
		fpr=fopen(argv[7],"r");
		if(fpr == NULL)
		{
			printf("file empty,exiting program");
			exit(0);
		}
		if(victim.cache_size!=0)
		{
			calculate_victim_spec();
			initialise_victim();
		}
		calculate_L1_spec();
		initialise_L1();
		if(L2.cache_size!=0)
		{
		calculate_L2_spec();
                initialise_L2();
		}
		while(!feof(fpr))
		//for(z=1; z<=122; z++)
		{
			fscanf(fpr," %c %x\n",&command,&address); 
			decode_add_L1(address);
			if(command=='r')
			{
				printf("\n %d. Address %x READ",z,address);
				read_cache_L1();
				L1_reads++;
			}
			if(command=='w')
			{
				printf("\n %d. Address %x WRITE",z,address);
				write_cache_L1();
				L1_writes++;
			}
			printf("\n");
		}
	}
	else
	{
		printf("No Arguments Passed");
	}
	printf("  ===== Simulator configuration =====\n");
	printf("  BLOCKSIZE:\t\t %d\n", L1.block_size);
	printf("  L1_SIZE:\t\t\t %d\n",L1.cache_size);
	printf("  L1_ASSOC:\t\t\t %d\n",L1.assoc);
	printf("  L2_SIZE:\t %d\n",L2.cache_size);
	printf("  L2_ASSOC:\t\t %d\n",L2.assoc);
	printf("  trace_file:\t\t %s\n",argv[7]);
	printf("   ===================================\n\n");
	printf("===== L1 contents =====\n");
	for(i=0;i<L1.number_sets;i++)
	{
		count=0;
		printf("set  %2d:   ",i);
		while(count!=(L1.assoc))
		{
		for(j=0;j<L1.assoc;j++)
		{
			if(L1.lru_counter[i][j]==count)
			{
			printf("%5x %c   ",L1.tagmatrix[i][j],L1.dirtybit[i][j]);
			count++;
			}
		}}
	printf("\n");
	}
	if(victim.cache_size!=0)
	{
	printf("===== Victim contents =====\n");
	for(i=0;i<victim.number_blocks;i++)
	{
		printf("%5x %c   ",victim.tagmatrix[i],victim.dirtybit[i]);
	}
	}
	if(L2.cache_size!=0)
	{
	printf("\n===== L2 contents =====\n");
	for(i=0;i<L2.number_sets;i++)
	{
		k=0;
		printf("set  %2d:   ",i);
		while(k!=(L2.assoc))
		{
		for(j=0;j<L2.assoc;j++)
		{
			if(L2.lru_counter[i][j]==k)
			{
			printf("%5x %c   ",L2.tagmatrix[i][j],L2.dirtybit[i][j]);
			k++;
			}
		}}
	printf("\n");
	}
	}
	printf("\n   ====== Simulation results (raw) ======\n");
	printf("   a. number of L1 reads:\t%d     \n",L1_reads);
	printf("   b. number of L1 read misses:\t%d     \n",cache_read_misses_L1);
	printf("   c. number of L1 writes:\t%d     \n",L1_writes);
	printf("   d. number of L1 write misses:\t%d     \n",cache_write_misses_L1);
	//printf("   e. L1 Miss rate:\t\t%.4f     \n",miss_rate);
	printf("   a. number of swaps:\t%d     \n",swaps);
	printf("   a. number of victim cache writeback:\t%d     \n",victim_writeback);
	printf("   a. number of L2 reads:\t%d     \n",L2_reads);
	printf("   b. number of L2 read misses:\t%d     \n",cache_read_misses_L2);
	printf("   c. number of L2 writes:\t%d     \n",L2_writes);
	printf("   d. number of L2 write misses:\t%d     \n",cache_write_misses_L2);
	printf("   f. number of L2 writebacks:\t%d     \n",write_backs_L2);
	//printf("   g. total memory traffic:\t%d     \n",(cache_read_misses+cache_write_misses+write_backs));
	//printf("   \n==== Simulation results (performance) ====\n");
	//printf("   1. average access time:\t%.4f ns\n",aat);*/
}
void read_cache_L1()
{
	int i,x=0;
	for(i=0;i<L1.assoc;i++)
	{
		if(L1.tagmatrix[L1.index][i]==L1.tag && L1.validbit[L1.index][i]==1) // read hit
		{
			x=i;
			printf("\n HIT L1");
			break;
		}
	}
	if(i==L1.assoc) //miss
	{
		printf("\n MISS L1");
		x=what_to_replace_L1(L1.index,L1.assoc);
		load_cache_address_L1(x,L1.tag,L1.index,L1.assoc);
		//cache_read_misses_L1++;
	} 
	update_lru_counter_L1(x,L1.index,L1.assoc);
}
int what_to_replace_L1(int index,int assoc)
{
	int i,x=0;
	for(i=0;i<assoc;i++)
	{
		if(L1.lru_counter[index][i]==assoc-1)
		{
			x=i;
			break;			
		}
	}
	return x;
}
void load_cache_address_L1(int position,int tag,int index,int assoc)
{
	int i,address_swap,dirty,address,writeback_address;
	address=tag;
	address=address<<(L1.index_bits+L1.block_offset_bits);
	address=(address+(L1.index<<L1.block_offset_bits));
	//address=address<<L1.block_offset_bits;
	for(i=0;i<assoc;i++)
	{
		if(i==position)
		{
			if(victim.cache_size!=0)
			{
				search_victim(address);
				switch(flag)
				{
					case 0:
					swaps++;
					address_swap=L1.tagmatrix[index][i]; // copies the tag which is to be replaced
					address_swap=address_swap<<(L1.index_bits+L1.block_offset_bits);//constructs address from the tag
					address_swap=(address_swap+(L1.index<<L1.block_offset_bits));
					//address_swap=address_swap<<L1.block_offset_bits;
					L1.tagmatrix[index][i]=tag;
					L1.validbit[index][i]=1;
					if(L1.dirtybit[index][i]=='D')
					dirty=1;
					else
					dirty=0;
					L1.dirtybit[index][i]='D';
					write_victim(address_swap,dirty);
					break;

					case 1:
					swaps++;
					address_swap=L1.tagmatrix[index][i]; // copies the tag which is to be replaced
					address_swap=address_swap<<(L1.index_bits+L1.block_offset_bits);//constructs address from the tag
					address_swap=(address_swap+(L1.index<<L1.block_offset_bits));
					//address_swap=address_swap<<L1.block_offset_bits;
					L1.tagmatrix[index][i]=tag;
					L1.validbit[index][i]=1;
					if(L1.dirtybit[index][i]=='D')
					dirty=1;
					else
					dirty=0;
					L1.dirtybit[index][i]=' ';
					write_victim(address_swap,dirty);
					break;
					
					case 2:
					cache_read_misses_L1++;
					if(L2.cache_size!=0)
					{
						read_cache_L2(address); L2_reads++; }
						if(L1.validbit[index][i]==0)
						{
							
							L1.tagmatrix[index][i]=L1.tag;
							L1.validbit[index][i]=1;
						}
						else
						{
								address_swap=L1.tagmatrix[index][i]; // copies the tag which is to be replaced
								address_swap=address_swap<<(L1.index_bits+L1.block_offset_bits);//constructs address from the tag
								address_swap=(address_swap+(L1.index<<L1.block_offset_bits));
								//address_swap=address_swap<<L1.block_offset_bits;
                                                        if(L1.dirtybit[index][i]=='D')
								dirty=1;
								else
								dirty=0;
								write_victim_when_victim_miss(address_swap,dirty);
								
							L1.tagmatrix[index][i]=tag;
							L1.dirtybit[index][i]=' ';
							L1.validbit[index][i]=1;
							
						}
					
					break;
					default:;
				}
			}
			else //victim not present
			{
				if(L1.dirtybit[index][i]=='D')
				{
					//write_backs_L2++;
					//printf("read miss address: %x \n", address);
					printf("\n L1 WRITEBACK");
					writeback_address=L1.tagmatrix[index][i];
					writeback_address=encode_writeback_address(writeback_address);
					//printf("encoded address: %x \n",writeback_address);
					write_cache_L2(writeback_address);
					L2_writes++;
				}
				read_cache_L2(address);
				L1.tagmatrix[index][i]=tag;
				L1.dirtybit[index][i]=' ';
				L1.validbit[index][i]=1;
				L2_reads++;
			}
		break;}
	}
}
void search_victim(int address)
{
	int i,x=0;
	decode_add_victim(address);
	for(i=0;i<victim.number_blocks;i++)
	{	printf("\nvc_store: %x given_tag: %x\n", victim.tagmatrix[i],victim.tag);
		if(victim.tagmatrix[i]==victim.tag && victim.dirtybit[i]=='D')
		{
			flag=0;
			x=i;
			update_lru_counter_victim(x);
			printf("\nHIT VICTIM CACHE,SWAP");
			break;
		}
		else if(victim.tagmatrix[i]==victim.tag && victim.dirtybit[i]==' ')
		{
			flag=1;
			x=i;
			update_lru_counter_victim(x);
			printf("\nHIT VICTIM CACHE,SWAP");
			break;
		}
		else
		{
			flag=2;
			printf("\nMISS VICTIM CACHE\n");
		}
	}
}
void write_victim(int address_swap,int dirty)
{
	int i,x;
	for(i=0;i<victim.number_blocks;i++)
	{
		if(victim.tagmatrix[i]==victim.tag)
		{	
			address_swap=address_swap>>victim.block_offset_bits;
			victim.tagmatrix[i]=address_swap;
			x=i;
			if(dirty==1)
			victim.dirtybit[i]='D';
			else
			victim.dirtybit[i]=' ';
                     break;
		}
	}
	//update_lru_counter_victim(x);
}
void write_victim_when_victim_miss(int address_swap,int dirty)
{
	int i,x=0,address;
	for(i=0;i<victim.number_blocks;i++)
	{
		if(victim.lru_counter[i]==(victim.number_blocks-1))
		{
			if(victim.dirtybit[i]=='D')
			{	
				victim_writeback++;
				address=(victim.tagmatrix[i]<<victim.block_offset_bits);
				if(L2.cache_size!=0)
				write_cache_L2(address);
				victim_writeback++;
				x=i;
				victim.tagmatrix[i]=(address_swap>>victim.block_offset_bits);
				if(dirty==1)
				victim.dirtybit[i]='D';
				else
				victim.dirtybit[i]=' ';
				break;
			}
			else
			{
				victim.tagmatrix[i]=(address_swap>>victim.block_offset_bits);
				victim.validbit[i]=1;
				x=i;
				if(dirty==1)
				victim.dirtybit[i]='D';
				else
				victim.dirtybit[i]=' ';
				break;
			}
		}
	}
	update_lru_counter_victim(x);
}
void update_lru_counter_victim(int position)
{
	int compare=victim.lru_counter[position];
	int i;
	for(i=0;i<victim.number_blocks;i++)
	{
		if(victim.lru_counter[i]<compare)
		victim.lru_counter[i]++;
	}
	victim.lru_counter[position]=0;
}
	
void write_cache_L1()
{
	int i,x=0;
	for(i=0;i<L1.assoc;i++)
	{
		if(L1.tagmatrix[L1.index][i]==L1.tag && L1.validbit[L1.index][i]==1)
		{
			L1.dirtybit[L1.index][i]='D';
			x=i;
			printf("\n HIT L1");
			break;
		}
	}
	if(i==L1.assoc)
	{
		printf("\n MISS L1");
		x=what_to_replace_L1(L1.index,L1.assoc);
		load_cache_wbwa_miss_L1(x,L1.tag,L1.index,L1.assoc);
	}
	update_lru_counter_L1(x,L1.index,L1.assoc);
}
void load_cache_wbwa_miss_L1(int position,int tag,int index,int assoc)
{
	int i,address,address_swap,dirty,writeback_address;
	address=L1.tag;
	address=address<<(L1.block_offset_bits+L1.index_bits);
	address=(address+(L1.index<<L1.block_offset_bits));
	//address=address<<L1.block_offset_bits;
	for(i=0;i<assoc;i++)
	{
		if(i==position)
		{
			if(victim.cache_size!=0)
			{
				search_victim(address);
				switch(flag)
				{
					case 0:
					address_swap=L1.tagmatrix[index][i]; // copies the tag which is to be replaced
					address_swap=address_swap<<(L1.block_offset_bits+L1.index_bits);
					address_swap=(address_swap+(L1.index<<L1.block_offset_bits));
					//address_swap=address_swap<<L1.block_offset_bits;
					L1.tagmatrix[index][i]=L1.tag;
					L1.validbit[index][i]=1;
					if(L1.dirtybit[index][i]=='D')
					dirty=1;
					else
					dirty=0;
					L1.dirtybit[index][i]='D';
					write_victim(address_swap,dirty);
					swaps++;
					break;
					
					case 1:
					address_swap=L1.tagmatrix[index][i]; // copies the tag which is to be replaced
					address_swap=address_swap<<(L1.block_offset_bits+L1.index_bits);
					address_swap=(address_swap+(L1.index<<L1.block_offset_bits));
					//address_swap=address_swap<<L1.block_offset_bits;
					L1.tagmatrix[index][i]=L1.tag;
					L1.validbit[index][i]=1;
					if(L1.dirtybit[index][i]=='D')
					dirty=1;
					else
					dirty=0;
					L1.dirtybit[index][i]='D';
					write_victim(address_swap,dirty);
					swaps++;
					break;

					case 2:
					cache_write_misses_L1++;
					if(L2.cache_size!=0)
					{
						read_cache_L2(address); L2_reads++; }
						if(L1.validbit[index][i]==0)
						{
							
							L1.tagmatrix[index][i]=L1.tag;
							L1.validbit[index][i]=1;
						}
						else
						{
								address_swap=L1.tagmatrix[index][i]; // copies the tag which is to be replaced
								address_swap=address_swap<<(L1.index_bits+L1.block_offset_bits);//constructs address from the tag
								address_swap=(address_swap+(L1.index<<L1.block_offset_bits));
								//address_swap=address_swap<<L1.block_offset_bits;
                                                        if(L1.dirtybit[index][i]=='D')
								dirty=1;
								else
								dirty=0;
								write_victim_when_victim_miss(address_swap,dirty);
								
							L1.tagmatrix[index][i]=tag;
							L1.dirtybit[index][i]='D';
							L1.validbit[index][i]=1;
							
						}

					break;
					default:;
				}
			}
			else
			{
				if(L1.dirtybit[index][i]=='D')
				{
					//write_backs_L2++;
					//printf("\n Evicted Tag :: %x Index :: %x",L1.tagmatrix[index][i],index);
					printf("\n L1 WRITEBACK");
					writeback_address=L1.tagmatrix[index][i];
					writeback_address=encode_writeback_address(writeback_address);
					write_cache_L2(writeback_address);
					L2_writes++;
				}
				read_cache_L2(address);
				L1.dirtybit[index][i]='D';
				L1.validbit[index][i]=1;
				L1.tagmatrix[index][i]=tag;
				L2_reads++;
			}
		break;}
	}
}
void update_lru_counter_L1(int position,int index,int assoc)
{
	int i,compare;
	compare=L1.lru_counter[index][position];
	
	for(i=0;i<assoc;i++)
	{
		if(L1.lru_counter[index][i]<compare)
		{
			L1.lru_counter[index][i]++;
		}
	}
	L1.lru_counter[index][position]=0;
}
int encode_writeback_address(int writeback_address)
{
	writeback_address=writeback_address<<L1.index_bits;
	writeback_address=writeback_address+L1.index;
	writeback_address=writeback_address<<L1.block_offset_bits;
	//writeback_address=writeback_address<<L1.block_offset_bits;
	return writeback_address;
	//printf("\n Reconstructed Address :: %x",address);
}
//read write functions of L2 begin from here
void read_cache_L2(int address)
{
	int i,x=0;
	printf("\n READ FROM L2 %x\n\n",address);
	decode_add_L2(address);
	printf("\n L2_tag %x L2_index %x L2_assoc %x\n\n",L2.tag,L2.index,L2.assoc);
	for(i=0;i<L2.assoc;i++)
	{
	//assert(L2.tagmatrix[25]!=NULL);
		if(L2.tagmatrix[L2.index][i]==L2.tag && L2.validbit[L2.index][i]==1) // read hit
		{
			x=i;
			printf("\n HIT L2\n\n");
			break;
		}
	}
	if(i==L2.assoc) //miss
	{
		printf("\n MISS L2\n\n");
		x=what_to_replace_L2(L2.index,L2.assoc);
		load_cache_address_L2(x,L2.tag,L2.index,L2.assoc);
		cache_read_misses_L2++;
	} 
	update_lru_counter_L2(x,L2.index,L2.assoc);
}
int what_to_replace_L2(int index,int assoc)
{
	int i,x=0;
	for(i=0;i<assoc;i++)
	{
		if(L2.lru_counter[index][i]==assoc-1)
		{
			x=i;
			break;
		}
	}
	return x;
}
void load_cache_address_L2(int position,int tag,int index,int assoc)
{
	int i;
	for(i=0;i<assoc;i++)
	{
		if(i==position)
		{
			if(L2.dirtybit[index][i]=='D')
			{
				write_backs_L2++;
				printf("\n L2 WRITEBACK");
			}
			L2.tagmatrix[index][i]=tag;
			L2.dirtybit[index][i]=' ';
			L2.validbit[index][i]=1;
		}
	}
}
void write_cache_L2(int address)
{
	decode_add_L2(address);
	int i,x=0;
	for(i=0;i<L2.assoc;i++)
	{
		if(L2.tagmatrix[L2.index][i]==L2.tag && L2.validbit[L2.index][i]==1)
		{
			L2.dirtybit[L2.index][i]='D';
			x=i;
			printf("\n HIT L2");
			break;
		}
	}
	if(i==L2.assoc)
	{
		printf("\n MISS L2");
		x=what_to_replace_L2(L2.index,L2.assoc);
		load_cache_wbwa_miss_L2(x,L2.tag,L2.index,L2.assoc);
		cache_write_misses_L2++;
	}
	update_lru_counter_L2(x,L2.index,L2.assoc);
}
void load_cache_wbwa_miss_L2(int position,int tag,int index,int assoc)
{
	int i;
	for(i=0;i<assoc;i++)
	{
		if(i==position)
		{
			if(L2.dirtybit[index][i]=='D')
			{
				write_backs_L2++;
				printf("\n L2 WRITEBACK");
			}
			L2.dirtybit[index][i]='D';
			L2.validbit[index][i]=1;
			L2.tagmatrix[index][i]=tag;
		}
	}
}
void update_lru_counter_L2(int position,int index,int assoc)
{
	int i,compare;
	compare=L2.lru_counter[index][position];
	
	for(i=0;i<assoc;i++)
	{
		if(L2.lru_counter[index][i]<compare)
		{
			L2.lru_counter[index][i]++;
		}
	}
	L2.lru_counter[index][position]=0;
}
