def generate_condition_morethan(inputs,a): 
	# generate all subsets of inputs with size a
	b = len(inputs)
	indices = list(range(a))
	condition = "(("
	while True:
		i = a-1
		while i >= 0 and indices[i] == i + b - a:
			i -= 1

		strcondition =  str(inputs[indices[0]])

		for g in indices[1:]:
			strcondition += " and " + str(inputs[g])
		condition += strcondition + ") or ("

		if i < 0:
			return condition[:-5] + ")"

		indices[i] += 1


		for j in range(i+1,a):
			indices[j] = indices[j-1]+1


def invert_general(n): # n is the number of not gates, it can invert 2^n-1 inputs

	num_inputs = 2**n
	print(f'{num_inputs-1} inputs')

	inputs = [chr(i+65) for i in range(num_inputs-1)]
	outputs = ["" for i in range(num_inputs-1)]

	# 1+, 2+, 3+, 4+... signals high
	conditions = [generate_condition_morethan(inputs,i) for i in range(1,num_inputs)]

	branches = [conditions[num_inputs // 2 - 1]]
	halfplus = conditions[num_inputs // 2 - 1]
	tree = ["not" + halfplus,halfplus] 

	for i in range(2,n+1):
		treesize = len(tree)
		newtree = [0 for j in range(treesize*2)]
		start = num_inputs // (2**i)  
		pluses = conditions[start-1::2*start]
		idx = 1
		for plus, condition in zip(pluses, tree): 
			newtree[idx] = f'({plus} and {condition})'
			idx += 2
		groupcondition = "".join(condition + " or " for condition in newtree if condition != 0)[:-4]
		notgroupcondition = "not (" + groupcondition + ")"  
		idx = 0
		for condition in tree:
			newtree[idx] = f'({notgroupcondition} and {condition})'
			idx += 2
		tree = newtree

	outputs = [tree[0] for i in range(num_inputs-1)]
	for i in range(num_inputs-1):
		exclusive_list = inputs[:i] + inputs[i+1:]
		for j in range(1,num_inputs-1):
			outputs[i] += "or (" + tree[j] + "and" + generate_condition_morethan(exclusive_list,j) + ")"

	return outputs
	

def evaluate_logic(logic,inputs):
	for i in range(inputs):
		for j in range(inputs):
			logic[i] = logic[i].replace(chr(65+j),f'strbin[{j}]')
	
	for i in range(2**inputs):
		strbin= [int(j) for j in bin(i)[2:].rjust(inputs, '0')]

		newline = []
		for i in logic:
			newline += [int(eval(i))]

		print("input: ",*strbin, " output: ",*newline) 


# change this
not_gates = 4

logic = invert_general(not_gates)
inputs = 2**not_gates - 1

# comment this out if its running slow
evaluate_logic(logic,inputs)

try:
	f = open("output.txt","x")
except:
	f = open("output.txt","w")
for i,circuit in enumerate(logic):
	f.write(f'not {chr(i+65)} : {circuit} \n')

