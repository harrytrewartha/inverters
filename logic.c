#include "main.h"

int n_choose_r(int n,int r){
	u64 topfrac = 1;
	u64 botfrac = 1;
	if (r > n - r) r = n - r;
	for(int a =n-r+1,b=1;b<=r;){
		topfrac *= a++;
		botfrac *= b++;
	}
	return (int)(topfrac / botfrac);
}

int max(int a,int b){
	return a<b?b:a;
}

void generate_subsets_of_size(int n, int subset_size, int** subsets) {
    int* indices = malloc(sizeof(int) * subset_size);
    
    // Initialize indices to [0, 1, 2, ..., subset_size-1]
    for (int i = 0; i < subset_size; i++) {
        indices[i] = i;
    }
    
    int idx = 0;
    
    for (;;) {

    	memcpy(subsets[idx],indices,sizeof(int)*subset_size);
        idx++;

        int i = subset_size - 1;
        while (i >= 0 && (indices[i] == i + n - subset_size)) {
            i--;
        }
        
        if (i < 0) {
            break;
        }
        
        indices[i]++;
        
        for (int j = i + 1; j < subset_size; j++) {
            indices[j] = indices[j - 1] + 1;
        }
    }
    
    free(indices);
}

void add_layers(Circuit* circuit){
	int layer_idx = circuit->max_layers;
	circuit->max_layers*=2;
	circuit->gates = realloc(circuit->gates,sizeof(Gate*)*circuit->max_layers);
	circuit->layer_sizes = realloc(circuit->layer_sizes,sizeof(int)*circuit->max_layers);

	for(int i = layer_idx-1;i<circuit->max_layers;i++){
		circuit->gates[i] = calloc(DEFAULT_LAYER_SIZE,sizeof(Gate));
		circuit->layer_sizes[i] = 0;
	}
}

Gate* add_gate(Circuit* circuit,Gate* input1, Gate* input2,Gate_type type){
	Gate new_gate;

	int layer = 0;
	// check if this gate already exists
	switch (type){
		case SWITCH:
			layer=-1;
			break;
		case NOT:
			layer = input1->layer;
			break;
		case OR:
		case AND:
			layer = max(input1->layer,input2->layer);
			for(int i=0;i<circuit->max_layers && circuit->layer_sizes[i]!=0;i++){
				for(int j=0;j<circuit->layer_sizes[i];j++){
					Gate curr_gate = circuit->gates[i][j];
					if (((input1==curr_gate.input1 && input2==curr_gate.input2) ||
				   		(input1==curr_gate.input2 && input2==curr_gate.input1)) &&
				   		(type==curr_gate.type)){
						return circuit->gates[i]+j;
					}
				}
			}
			break;
	
	}

	new_gate.type = type;


	while (true){
		layer++;
		if (layer >= circuit->max_layers){
			add_layers(circuit);
		}
		if (circuit->layer_sizes[layer] < DEFAULT_LAYER_SIZE)break;
	}
	new_gate.layer = layer;
	new_gate.input1 = input1;
	new_gate.input2 = input2;
	new_gate.output = false;

	Gate* gateptr = &circuit->gates[layer][circuit->layer_sizes[layer]];
	*gateptr = new_gate;

	circuit->layer_sizes[layer]++;
	return gateptr;
}



// turn a gate with multiple inputs into a chain of gates with 2 inputs
// input is array of pointers
Gate* gate_chain(Gate** gates, Circuit* circuit, int num_inputs,Gate_type type){
	if (num_inputs == 1)return gates[0];
	int new_arr_size = num_inputs >> 1;
	Gate** new_gate_arr = malloc(sizeof(Gate*)*(new_arr_size + (num_inputs & 1)));
	int a=0,b=0;
	for(;a<new_arr_size;a++){
		new_gate_arr[a] = add_gate(circuit,gates[b++],gates[b++],type);
	}
	if (num_inputs & 1){
		new_gate_arr[new_arr_size]=gates[num_inputs-1];
		new_arr_size++;
	} 
	Gate* result =  gate_chain(new_gate_arr,circuit,new_arr_size,type);
	free(new_gate_arr);
	return result;
}

void generate_exclusive_subsets(Circuit* circuit,Gate** signal_arr){
	int n = circuit->input_count-1;
	Gate* exclusive_inputs[n];
	int*** subset_indices = malloc(sizeof(int**)*n); // number of sizes of subsets
	for(int i = 0;i<n;i++){
		int num_subsets = n_choose_r(n,i+1); // num subsets of cardinality i+1
		subset_indices[i] = malloc(sizeof(int*)*num_subsets);
		for(int j = 0;j<num_subsets;j++){
			subset_indices[i][j] = malloc(sizeof(int) * (i+1)); // cardinality i + 1
		}
	}

	for(int i = 0;i<n;i++){
		generate_subsets_of_size(n,i+1,subset_indices[i]);
	}

	for(int i = 0;i<circuit->input_count;i++){
		// generate set without current output
		for(int a,b = a = 0;a<circuit->input_count-1;a++){
			exclusive_inputs[a] = &circuit->gates[0][b++];
			if (b == i)b++;
		}

		Gate* clauses[circuit->input_count];
		clauses[0] = signal_arr[0];
		for(int subset_size = 1;subset_size<n+1;subset_size++){
			int num_subsets = n_choose_r(n,subset_size);
			Gate* subsets[num_subsets] = {};
			Gate* subset[subset_size] = {};
			for(int k = 0;k<num_subsets;k++){
				for(int idx = 0;idx<subset_size;idx++){
					subset[idx] = exclusive_inputs[subset_indices[subset_size-1][k][idx]];
				}
				subsets[k] = gate_chain(subset,circuit,subset_size,AND);
			}
			Gate* num_signals = signal_arr[subset_size];
			clauses[subset_size] = add_gate(circuit,gate_chain(subsets,circuit,num_subsets,OR),num_signals,AND);

		}
		gate_chain(clauses,circuit,circuit->input_count,OR);
	}
	for(int i=0;i<n;i++){
		for(int j=0;j<n_choose_r(n,i+1);j++){
			free(subset_indices[i][j]);
		}
		free(subset_indices[i]);
	}
	free(subset_indices);
}


Gate* generate_condition_morethan(Circuit* circuit, int subset_size){
	// add all subsets of cardinality a
	// add the gates directly


	int set_size = circuit->input_count;

	int num_subsets = n_choose_r(set_size,subset_size);
	Gate* subset[subset_size];
	Gate** subsets = malloc(sizeof(Gate*)*num_subsets);
	int subset_idx = 0;
	Gate* inputs = circuit->gates[0];
	int indices[subset_size];
	for(int i = 0;i<subset_size;i++){
		indices[i]=i;
	}



	for(;;){
		int i = subset_size - 1;
		while (i>= 0 && (indices[i] == i + set_size - subset_size))i--;
		
		for(int j = 0;j<subset_size;j++){
			subset[j] = inputs+indices[j];
		}
		subsets[subset_idx++] =	gate_chain(subset,circuit,subset_size,AND);

		
		if (i < 0){
			Gate* return_gate = gate_chain(subsets,circuit,num_subsets,OR);
			free(subsets);
			return return_gate;
		}
		indices[i]++;
		for(int j = i+1;j<subset_size;j++){
			indices[j]=indices[j-1]+1;
		}
	}
}

Circuit* create_circuit(int num_inputs){
	Circuit* circuit = malloc(sizeof(Circuit));

	circuit->max_layers = DEFAULT_NUM_LAYERS;
	circuit->input_count = num_inputs;

	circuit->layer_sizes = calloc(DEFAULT_NUM_LAYERS,sizeof(int));
	circuit->gates = calloc(DEFAULT_NUM_LAYERS,sizeof(Gate*));

	for(int i = 0;i<DEFAULT_NUM_LAYERS;i++){
		circuit->gates[i] = calloc(DEFAULT_LAYER_SIZE,sizeof(Gate));
	}

	for(int i = 0;i<num_inputs;i++){
		add_gate(circuit,NULL,NULL,SWITCH);
	}


	return circuit;
}

void construct_circuit(Circuit* circuit, int num_nots){

	Gate* halfplus = generate_condition_morethan(circuit,circuit->input_count/2);


	Gate** tree = malloc(sizeof(Gate*)*2);
	tree[0] = add_gate(circuit,halfplus,NULL,NOT);
	tree[1] = halfplus;

	int treesize = 1;
	for(int i = 2;i<num_nots+1;i++){
		treesize<<=1;

		Gate** newtree = calloc(treesize*2,sizeof(Gate*));
		int start = (circuit->input_count+1) / (1<<i);
		Gate* pluses[treesize];
		int idx = 0;
		for(int j = start-1;j<circuit->input_count;j+=2*start){
			pluses[idx++] = generate_condition_morethan(circuit,j+1);
		}
		idx = 1;
		Gate* group1[treesize];
		for(int j = 0;j<treesize;j++){
			group1[j] = newtree[idx] = add_gate(circuit,pluses[j],tree[j],AND);
			idx+=2;
		}
		Gate* group_condition = gate_chain(group1,circuit,treesize,AND);
		Gate* not_group_condition = add_gate(circuit,group_condition,NULL,NOT);
		idx = 0;
		for(int j = 0;j<treesize;j++){
			newtree[idx]=add_gate(circuit,not_group_condition,tree[j],AND);
			idx += 2;
		}
		free(tree);
		tree = newtree;
	}
	generate_exclusive_subsets(circuit,tree);
}

void destroy_circuit(Circuit* circuit){
	for(int i = 0;i<circuit->max_layers;i++){
		free(circuit->gates[i]);	
	}
	free(circuit->gates);
	free(circuit->layer_sizes);
	free(circuit);
}