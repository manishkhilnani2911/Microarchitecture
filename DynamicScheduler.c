# include<stdio.h>
# include<stdlib.h>
# include<math.h>
# include<vector>
using namespace std;
int **register_file,instruction_number = 0,check = 0;
int fetch_bandwidth,scheduling_window_size,fetch_count=0,dispatch_count=0,schedule_count=0,execute_count=0,overall_cycle_count= 0;

int** make_register_file();
void fetch();
void dispatch();
void schedule();
void execute();
void issue();
void retire();
void free_register_file();
void count_state_cycles();
struct cache
{
unsigned long cache_size;
int block_size,assoc,read_misses,reads;
int number_blocks,number_sets,block_offset_bits,index_bits,tag_bits,tag,index,index_mask;
int **tagmatrix,**lru_counter,**validbit;
char **dirtybit;
};
cache L1,L2;
struct instruction{
	int destination,source1,tag1,tag2,source2,d_tag,pc,type,ex_time,address,ready_src_1,ready_src_2;
	int cycle_fetch,cycle_dispatch,cycle_schedule,cycle_execute,cycle_writeback,fetch_cycles,dispatch_cycles,schedule_cycles,writeback_cycles;
	char state;
};
instruction inst;
vector<instruction> rob;
vector<instruction>::iterator iter;
vector<instruction>::iterator r;
FILE *fr;
int** make_register_file()
{
	int** register_array;
    int i;
	register_array = (int**) malloc(128 * sizeof(int*));
    for (i = 0;i < 128; i++)
	    register_array[i] = (int*) malloc(2 * sizeof(int));
	for(i = 0;i < 128;i++){
		register_array[i][0] = 0;	
		register_array[i][1] = 1;
	}
    return register_array;
}

int main(int argc,char *argv[])
{
	register_file = make_register_file();
	scheduling_window_size = atoi(argv[1]);
	fetch_bandwidth = atoi(argv[2]);
	block_size=atoi(argv[3]);
	L1_size=atoi(argv[4]);
	L1_assoc=atoi(argv[5]);
	L2_size=atoi(argv[6]);
	L1_assoc=atoi(argv[7]);
	fr = fopen(argv[8],"r");	
 	while((check == 0) || (fetch_count + dispatch_count + schedule_count + execute_count != 0)){
		retire();
		issue();
		dispatch();
		fetch();
		count_state_cycles();
	}
	for(iter = rob.begin();iter != rob.end();iter++)
        printf("%d fu{%d} src{%d,%d} dst{%d} IF{%d,%d} ID{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d}\n",iter->d_tag,iter->type,iter->source1,iter->source2,iter->destination,iter->cycle_fetch,iter->fetch_cycles,iter->cycle_dispatch,iter->dispatch_cycles,iter->cycle_schedule,iter->schedule_cycles,iter->cycle_execute,iter->ex_time,iter->cycle_writeback,iter->writeback_cycles);
	printf("CONFIGURATION \n");
	printf(" superscalar bandwidth (N) = %d\n",fetch_bandwidth);
	printf(" dispatch queue size (2*N) = %d\n",2*fetch_bandwidth);
	printf(" schedule queue size (S)   = %d\n",scheduling_window_size);
	printf("RESULTS \n");
	printf(" number of instructions = %d\n",instruction_number);
	printf(" number of cycles       = %d\n",overall_cycle_count);
	float ipc = (float)(instruction_number)/(float)(overall_cycle_count);
	printf(" IPC\t\t\t= %0.2f\n",ipc);
}
void fetch()
{
	while( (fetch_count+dispatch_count < 2*fetch_bandwidth) && (fetch_count < fetch_bandwidth)){
		if(fscanf(fr,"%x %d %d %d %d %x",&inst.pc,&inst.type,&inst.destination,&inst.source1,&inst.source2,&inst.address)!= EOF){
			inst.d_tag = instruction_number;
			inst.state = 'f';
			inst.ex_time = 0;
			inst.fetch_cycles = 0;
			inst.dispatch_cycles = 0;
			inst.schedule_cycles = 0;
			inst.writeback_cycles=1;
			inst.cycle_fetch=overall_cycle_count;
			inst.cycle_dispatch=0;
			inst.cycle_schedule=0;
			inst.cycle_execute=0;
			inst.cycle_writeback=0;
			fetch_count++;
			rob.push_back(inst);
			instruction_number++;
		}
		else{
			check = 1;
			break;
		}
	}
	overall_cycle_count++;
}
void dispatch()
{
	for(iter = rob.begin();iter != rob.end();iter++){
		if(iter->state == 'd' && schedule_count < scheduling_window_size){
			iter->state = 's';
			iter->cycle_schedule = overall_cycle_count;
			schedule_count++;
			dispatch_count--;
			if(iter->source1 != -1){
				iter->ready_src_1 = register_file[iter->source1][1];
				iter->tag1 = register_file[iter->source1][0];		
			}
           	if(iter->source2 != -1){
				iter->ready_src_2 = register_file[iter->source2][1];
				iter->tag2 = register_file[iter->source2][0];
			}
			if(iter->destination != -1){
				register_file[iter->destination][1] = 0;
				register_file[iter->destination][0] = iter->d_tag;
			}
		}
	}
	for(iter = rob.begin();iter != rob.end();iter++){
		if(iter->state == 'f'){
			iter->state = 'd';
			iter->cycle_dispatch = overall_cycle_count;
			dispatch_count++;
			fetch_count--;
		}
	}	
}
void issue()
{
	int f = 0;
	for(iter = rob.begin();iter != rob.end();iter++){
        if(iter->state == 's' && f < fetch_bandwidth){
			if((iter->ready_src_1 == 1 || iter->source1 == -1) && (iter->ready_src_2 == 1 || iter->source2 == -1)){
				f++;
				iter->state = 'e';
				iter->cycle_execute = overall_cycle_count;
				schedule_count--;
				execute_count++;
			}
		}
	}
}
void retire()
{
	for(iter = rob.begin();iter != rob.end();iter++){
		if(iter->state == 'e'){
			iter->ex_time++;
			if(iter->type == 0 && iter->ex_time == 1){
				iter->cycle_writeback = overall_cycle_count;
				free_register_file();
				execute_count--;
				iter->state = 'x';
			}
			if(iter->type == 1 && iter->ex_time == 2){
				iter->cycle_writeback = overall_cycle_count;
				free_register_file();
				execute_count--;
				iter->state = 'x';
            }
			if(iter->type == 2 && iter->ex_time == 5){
				iter->cycle_writeback = overall_cycle_count;
				free_register_file();
				execute_count--;
				iter->state = 'x';
		    }
		}			  	
	}
}
void free_register_file()
{
	if(iter->destination != -1 && register_file[iter->destination][0] == iter->d_tag)
	register_file[iter->destination][1] = 1;
	for(r = rob.begin();r != rob.end();r++){
		if(r->state == 's' &&  iter->destination != -1){
			if(iter->d_tag == r->tag1) 
				r->ready_src_1 = 1;
 			if(iter->d_tag == r->tag2)
				r->ready_src_2 = 1;
		}
	}
}
void count_state_cycles()
{
	for(iter=rob.begin();iter!=rob.end();iter++){
		if(iter->state == 'f')
			iter->fetch_cycles++;
		if(iter->state=='d')
			iter->dispatch_cycles++;
		if(iter->state=='s')
			iter->schedule_cycles++;
	}
}


