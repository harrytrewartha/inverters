#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define DEFAULT_LAYER_SIZE 100
#define DEFAULT_NUM_LAYERS 16
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

typedef Vector2 Vec2;
typedef uint64_t u64;
typedef uint8_t u8;
typedef enum {
	AND,
	OR,
	NOT,
	SWITCH // pretend that switches are a type of gate
} Gate_type;

typedef struct Gate{
	Gate_type type;
	int layer;
	bool output;
	struct Gate* input1;
	struct Gate* input2;
} Gate;


typedef struct Circuit{
	// BFS queue
	Gate** gates;
	int max_layers;
	int input_count;
	int* layer_sizes;
} Circuit;

Gate* gate_chain(Gate** gates, Circuit* circuit, int num_inputs,Gate_type type);
void generate_exclusive_subsets(Circuit* circuit, Gate** signal_tree);
Gate* add_gate(Circuit* circuit,Gate* input1, Gate* input2,Gate_type type);
void generate_subsets_of_size(int n, int subset_size, int** subsets);