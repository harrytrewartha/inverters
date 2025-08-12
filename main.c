#include "main.h"



Wire_manager* create_wire_manager(Circuit* circuit){
	int num_gates = 0;
	for(int i = 0;i<circuit->max_layers && circuit->layer_sizes[i]>0;i++){
		num_gates += circuit->layer_sizes[i];
	}

	Wire_manager* wm = malloc(sizeof(Wire_manager));
	wm->wires = malloc(sizeof(Wire)*num_gates);
	wm->free_track = malloc(sizeof(int)*DEFAULT_NUM_TRACKS);
	wm->tracks = malloc(sizeof(Wire*)*DEFAULT_NUM_TRACKS);
	wm->free_track_idx = 0;
	wm->max_tracks = DEFAULT_NUM_TRACKS;
	for(int i = 0;i<DEFAULT_NUM_TRACKS;i++){
		wm->free_track[i] = i;
	}
}

void add_wire(Wire_manager* wm,Circuit* circuit,int layer,int idx){
	Wire new_wire = {0};
	new_wire.start_layer = layer;
	
	Gate* src = &circuit->gates[layer][idx];
	new_wire.source = (IVec2){layer,idx};
	for(int i = circuit->max_layers;i>layer+1;i--){
		for(int j = 0;j<circuit->layer_sizes[i];j++){
			Gate* curr_gate = &circuit->gates[i][j];
			if (curr_gate->input1 == src || curr_gate->input2 == src){
				new_wire.end_layer = i;
				goto end;
			}
		}
	}
	end:
	new_wire.track = (new_wire.start_layer+1!=new_wire.end_layer)?wm->free_track[wm->free_track_idx++]:-1;
	wm->wires[wm->num_wires++]=new_wire;
	// -1 means it never needs to go to the tracks, all routing can be done in the space between layers
}

void update_frees(Wire_manager* wm,int layer){
	for (int i = wm->highest_track_used;i>-1;i--){
		Wire* curr_wire = wm->tracks[i];
		if (curr_wire == NULL)continue;

		if (curr_wire->end_layer < layer){
			wm->free_track[--wm->free_track_idx] = i;
			wm->tracks[i] = NULL;
		}
	}
}

Wire* wire_stuff(Circuit* circuit,int num_layers){
	Wire_manager* wm = create_wire_manager(circuit);

	for(int i = 0;i<num_layers;i++){
		update_frees(wm,i);
		for(int j = 0;j<circuit->layer_sizes[i];j++){
			add_wire(wm,circuit,i,j);
		}
	}
}


RTex2D create_gates_tex(Circuit* circuit,int num_layers){
	RTex2D rtex; // no t-rex :(
	Tex2D and_tex = LoadTextureFromImage(LoadImage("assets/and.png"));
	Tex2D or_tex = LoadTextureFromImage(LoadImage("assets/or.png"));
	Tex2D not_tex = LoadTextureFromImage(LoadImage("assets/not.png"));
	Tex2D switch_tex = LoadTextureFromImage(LoadImage("assets/switch.png"));

	BeginTextureMode(rtex);
	ClearBackground(WHITE);
	for(int i = 0;i<num_layers;i++){
		for(int j = 0;j<circuit->layer_sizes[i];j++){
			Gate curr_gate = circuit->gates[i][j];
			Tex2D draw_tex;
			switch (curr_gate.type){
				case AND:
					draw_tex = and_tex;
					break;
				case OR:
					draw_tex = or_tex;
					break;
				case NOT:
					draw_tex = not_tex;
					break;
				case SWITCH:
					draw_tex = switch_tex;
					break;
			}
			Rectangle source = {0,0,draw_tex.width,draw_tex.height};
			int width = draw_tex.width/2;
			int height = draw_tex.height/2;
			Vec2 centre = {width/2,height/2};
			Gate* input = curr_gate.input1;
			Rectangle target = {50+i*300,50+j*150,width,height};
			DrawTexturePro(draw_tex,source,target,centre,0,WHITE);
		}
	}

	EndTextureMode();
	return rtex;
}

void update_wire_rtex(RTex2D* rtex,Wire_manager* wm,Circuit* circuit){
	BeginTextureMode(*rtex);
	ClearBackground(WHITE);
	float hue_step = 360.0f / (float)wm->num_wires;
	float hue = 0.0f;
	for(int i = 0;i<wm->num_wires;i++){

		Wire wire = wm->wires[i];
		float sat = circuit->gates[wire.source.x][wire.source.y].output?1.0f:0.5f;
		Colour wire_col = ColorFromHSV(hue,sat,1.0f);
		hue += hue_step;
		DrawWire(wire)
	}
	EndTextureMode();
}

void camera_stuff(Camera2D* cam){
	if (IsKeyDown(KEY_W))cam->target.y -= 10;
	if (IsKeyDown(KEY_A))cam->target.x -= 10;
	if (IsKeyDown(KEY_S))cam->target.y += 10;
	if (IsKeyDown(KEY_D))cam->target.x += 10;

	float wheel = GetMouseWheelMove();
	if (wheel != 0) {
		cam->zoom += wheel * .1;
		if (cam->zoom < 0.1f)cam->zoom = 0.1f;
	}
}

void update_circuit_state(Circuit* circuit){
	for(int i=1;i<circuit->max_layers;i++){
		for(int j=0;j<circuit->layer_sizes[i];j++){
			Gate curr_gate = circuit->gates[i][j];
			bool output=false;
			bool a = curr_gate.input1->output;
			bool b = curr_gate.input2->output;
			switch (curr_gate.type){
				case AND:
					output = a && b;
					break;
				case OR:
					output = a || b;
					break;
				case NOT:
					output = !a;
			}
			curr_gate.output = output
		}
	}
}

bool get_updates(Circuit* circuit){
	// check if clicked on switch
	if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))return false;
	Vec2 mp = GetMousePosition();
	if (50>=mp.x || mp.x>=100)return false;
	int height = (int)mp.y-50;
	if (height % 150 > TEX_HEIGHT)return false;

	int switch_pressed = height/150;
	if (switch_pressed > circuit->input_count)return false;
	circuit->gates[0][switch_pressed].output=true;
	update_circuit_state(circuit);
	return true;
}


void run_circuit(Circuit* circuit){
	InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"window");


	SetTargetFPS(60);

	int max_layer_width = 0;
	int num_layers = 0;

	

	while(circuit->layer_sizes[num_layers]!=0){
		num_layers++;
		if (circuit->layer_sizes[num_layers] > max_layer_width){
			max_layer_width=circuit->layer_sizes[num_layers];
		}
	}

	Wire_manager* wires = wire_stuff(num_layers,circuit);

	Camera2D cam = {0};
	cam.target = (Vec2){0.0f,0.0f};
	cam.offset = (Vec2){SCREEN_WIDTH/2,SCREEN_HEIGHT/2};
	cam.rotation = 0.0f;
	cam.zoom = 1.0f;
	RTex2D gate_rtex = create_gates_rtex(circuit,num_layers);

	Wire* wires = wire_stuff(circuit,num_layers);
	RTex2D wire_rtex;
	update_wire_rtex(&wire_rtex,wm,circuit);

	while (!WindowShouldClose()){
		BeginDrawing();
		ClearBackground(WHITE);
		camera_stuff(&cam);
		bool updated = get_updates();
		if (updated){
			update_wire_tex(&wire_rtex,wm);
		}

		//rtexes are stored upside down apparently
		DrawTextureRec(gate_rtex.texture,
			(Rectangle){0,0,gate_rtex.texture.width,-gate_rtex.texture.height},
			(Vec2){0,0},
			WHITE
		);
		DrawTextureRec(wire_rtex.texture,
			(Rectangle){0,0,wire_rtex.texture.width,-wire_rtex.texture.height},
			(Vec2){0,0},
			WHITE
		);

		EndDrawing();
	}
}



int main(){
	int num_not_gates = 3;
	int num_inputs = (1<<num_not_gates)-1;
	Circuit* circuit = create_circuit(num_inputs);
	construct_circuit(circuit,num_not_gates);
	printf("constructed circuit \n");
	run_circuit(circuit);

	destroy_circuit(circuit);
	return 0;
}