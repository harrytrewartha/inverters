#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_LAYER_SIZE 30
#define DEFAULT_NUM_LAYERS 16
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define DEFAULT_NUM_TRACKS 50
#define TEX_WIDTH 50
#define TEX_HEIGHT 50

typedef Vector2 Vec2;
typedef Texture2D Tex2D;
typedef RenderTexture2D RTex2D;
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

typedef struct Ivec2{
	int x,y;
} IVec2;

typedef struct Wire{
	IVec2 source;
	int start_layer;
	int end_layer;
	int track;
} Wire;

typedef struct Wire_manager{
	Wire* wires;
	Wire** tracks;
	int highest_track_used;
	int* free_track;
	int free_track_idx;
	int max_tracks;
	int num_wires;
} Wire_manager;


typedef struct Circuit{
	Gate** gates;
	int max_layers;
	int input_count;
	int* layer_sizes;
} Circuit;

Circuit* create_circuit(int num_inputs);
void construct_circuit(Circuit* circuit, int num_nots);
void destroy_circuit(Circuit* circuit);