// demo.h
// demo note data

// lead

OpdMidiPitch seqBass[BASS_NUMBER][SEQ_NUMBER][BASS_NOTES] = 
	{
		{
			{0, 0, 0, 0, 
			0, 0, 0, 0, 
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0, 
			0, 0, 0, 0, 
			0, 0, 0, 0,
			0, 0, 0, 0}
		,
			{43, 43, 0, 43, 
			43, 0, 43, 43, 
			43, 43, 0, 43, 
			43, 0, 43, 43,
			39, 39, 0, 39, 
			39, 0, 39, 39, 
			39, 39, 0, 39, 
			39, 0, 39, 39}
		,
			{41, 41, 0, 41, 
			41, 0, 41, 41, 
			41, 41, 0, 41, 
			41, 0, 41, 41,
			38, 38, 0, 38, 
			38, 0, 38, 38, 
			38, 38, 0, 38, 
			38, 0, 38, 38}
		,
			{39, 39, 0, 39, 
			39, 0, 39, 39, 
			39, 39, 0, 39, 
			39, 0, 39, 39,
			41, 41, 0, 41, 
			41, 0, 41, 41, 
			41, 41, 0, 41, 
			41, 0, 41, 41}
		,
			{38, 38, 0, 38, 
			38, 0, 38, 38, 
			38, 38, 0, 38, 
			38, 0, 38, 38,
			43, 43, 0, 43, 
			43, 0, 43, 43, 
			43, 43, 0, 43, 
			43, 0, 43, 43}
		}
	,
		{
			{0, 0, 0, 0, 
			0, 0, 0, 0, 
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0, 
			0, 0, 0, 0, 
			0, 0, 0, 0,
			0, 0, 0, 0}
		,
			{43, 43, 43, 43, 
			43, 43, 43, 43, 
			43, 43, 43, 43, 
			43, 43, 43, 43,
			39, 39, 39, 39, 
			39, 39, 39, 39, 
			39, 39, 39, 39, 
			39, 39, 39, 39}
		,
			{41, 41, 41, 41, 
			41, 41, 41, 41, 
			41, 41, 41, 41, 
			41, 41, 41, 41,
			38, 38, 38, 38, 
			38, 38, 38, 38, 
			38, 38, 38, 38, 
			38, 38, 38, 38}
		,
			{39, 39, 39, 39, 
			39, 39, 39, 39, 
			39, 39, 39, 39, 
			39, 39, 39, 39,
			41, 41, 41, 41, 
			41, 41, 41, 41, 
			41, 41, 41, 41, 
			41, 41, 41, 41}
		,
			{38, 38, 38, 38, 
			38, 38, 38, 38, 
			38, 38, 38, 38, 
			38, 38, 38, 38,
			43, 43, 43, 43, 
			43, 43, 43, 43, 
			43, 43, 43, 43, 
			43, 43, 43, 43}

		}
	,
		{
			{0, 0, 0, 0, 
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0}
		,
			{67, 0, 67, 67, 
			67, 67, 0, 67, 
			67, 67, 67, 67, 
			67, 67, 0, 67, 
			63, 0, 63, 63, 
			63, 63, 0, 63, 
			63, 63, 63, 63, 
			63, 63, 0, 63}
		,
			{65, 0, 65, 65, 
			65, 65, 0, 65, 
			65, 65, 65, 65, 
			65, 65, 0, 65, 
			62, 0, 62, 62, 
			62, 62, 0, 62, 
			62, 62, 62, 62, 
			62, 62, 0, 62}
		,
			{63, 0, 63, 63, 
			63, 63, 0, 63, 
			63, 63, 63, 63, 
			63, 63, 0, 63, 
			65, 0, 65, 65, 
			65, 65, 0, 65, 
			65, 65, 65, 65, 
			65, 65, 0, 65}
		,
			{69, 0, 69, 69, 
			69, 69, 0, 69, 
			69, 69, 69, 69, 
			69, 69, 0, 69, 
			70, 0, 70, 70, 
			70, 70, 0, 70, 
			70, 70, 70, 70, 
			70, 70, 0, 70}
		}
	};



// lead

OpdNote seqLead[LEAD_NUMBER][SEQ_NUMBER][LEAD_NOTES] =
	{
		{
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		,
			{{0, t1}, {75, t4}, {74, t4}, {69, t4}, {70, t4}, 
			{72, t2}, {74, t1}, {0, t2}}
		,
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		,
			{{75, t2}, {0, t2}, {74, t2}, {69, t4}, {70, t4},
			{72, t2}, {74, t1}, {0, t2}}
		,
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		}
	,
		{
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		,
			{{57, t4}, {62, t4}, {0, t2}, {0, t1},
			{55, t4}, {62, t4}, {0, t2}, {0, t1}}
		,
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		,
			{{58, t4}, {63, t4}, {0, t2}, {57, t4}, {60, t4}, {0, t2},
			{57, t4}, {60, t4}, {0, t2}, {58, t4}, {62, t4}, {0, t2}}
		,
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		}
	,
		{
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		,
			{{0, t4}, {0, t8}, {67, t8}, {69, t8}, {67, t8}, {69, t8}, {67, t16}, {70, t8}, {0, t16}, {0, t8}, {0, t2}, {0, t8}, {67, t8}, {69, t8}, {67, t16}, {69, t8}, {0, t16}, {0, t2}, {67, t8}, {69, t8}, {67, t8}, {69, t8}, {67, t8}, {69, t16}, {70, t8}, {69, t8}, {0, t16}, {0, t8}}
		,
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		,
			{{0, t4}, {0, t8}, {67, t8}, {69, t8}, {67, t8}, {69, t8}, {67, t16}, {70, t8}, {0, t16}, {0, t8}, {0, t2}, {0, t8}, {67, t8}, {69, t8}, {67, t16}, {69, t8}, {0, t16}, {0, t2}, {67, t8}, {69, t8}, {67, t8}, {69, t8}, {67, t8}, {69, t16}, {70, t8}, {69, t8}, {0, t16}, {0, t8}}
		,
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		}
	,
		{
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		,
			{{0, t8}, {62, t8}, {0, t16}, {62, t8}, {0, t16}, {62, t16}, {62, t8}, {62, t16}, {0, t16}, {62, t16}, {0, t8},
			{0, t8}, {63, t8}, {0, t16}, {63, t8}, {0, t16}, {63, t16}, {63, t8}, {63, t16}, {0, t16}, {63, t16}, {0, t8},
			{0, t8}, {63, t8}, {0, t16}, {63, t8}, {0, t16}, {63, t16}, {62, t8}, {60, t16}, {0, t16}, {60, t16}, {0, t8},
			{0, t8}, {62, t8}, {0, t16}, {62, t8}, {0, t16}, {62, t16}, {62, t8}, {62, t16}, {0, t16}, {62, t16}, {0, t8}}
		,
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		,
			{{0, t8}, {63, t8}, {0, t16}, {63, t8}, {0, t16}, {63, t16}, {63, t8}, {63, t16}, {0, t16}, {63, t16}, {0, t8},
			{0, t8}, {60, t8}, {0, t16}, {60, t8}, {0, t16}, {60, t16}, {60, t8}, {60, t16}, {0, t16}, {60, t16}, {0, t8},
			{0, t8}, {62, t8}, {0, t16}, {62, t8}, {0, t16}, {62, t16}, {62, t8}, {62, t16}, {0, t16}, {62, t16}, {0, t8},
			{0, t8}, {62, t8}, {0, t16}, {62, t8}, {0, t16}, {62, t16}, {62, t8}, {62, t16}, {0, t16}, {62, t16}, {0, t8}}
		,
			{{0, t1}, {0, t1}, {0, t1}, {0, t1}}
		}
	};
uint8_t seqLeadLen[LEAD_NUMBER][SEQ_NUMBER] = {{4, 8, 4, 8, 4}, {4, 8, 4, 12, 4}, {4, 28, 4, 28, 4}, {4, 44, 4, 44, 4}};



// drums

uint8_t seqDrum[SEQ_NUMBER][DRUM_NOTES] =
	{
		{0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0}
	,
		{DRUM_KICK + DRUM_HHCLOSED, 0, DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED, 0, DRUM_HHOPEN, DRUM_KICK,
		DRUM_HHCLOSED, 0, DRUM_KICK + DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED, 0, DRUM_HHOPEN, 0,

		DRUM_KICK + DRUM_HHCLOSED, 0, DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED, 0, DRUM_HHOPEN, DRUM_KICK,
		DRUM_HHCLOSED, 0, DRUM_KICK + DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED + DRUM_CLAP, 0, DRUM_HHOPEN, DRUM_KICK}
	,
		{DRUM_KICK + DRUM_HHCLOSED, 0, DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED, 0, DRUM_HHOPEN, DRUM_KICK,
		DRUM_HHCLOSED, 0, DRUM_KICK + DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED, 0, DRUM_HHOPEN, 0,

		DRUM_KICK + DRUM_HHCLOSED, 0, DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED, 0, DRUM_HHOPEN, DRUM_KICK,
		DRUM_HHCLOSED, 0, DRUM_KICK + DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED + DRUM_CLAP, 0, DRUM_SNARE + DRUM_HHOPEN, DRUM_SNARE + DRUM_KICK}
	,
		{DRUM_KICK + DRUM_HHCLOSED + DRUM_CRASH, 0, DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED, 0, DRUM_HHOPEN, DRUM_KICK,
		DRUM_HHCLOSED, 0, DRUM_KICK + DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED + DRUM_CLAP, 0, DRUM_HHOPEN, 0,

		DRUM_KICK + DRUM_HHCLOSED, 0, DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED, 0, DRUM_HHOPEN, DRUM_KICK,
		DRUM_HHCLOSED, 0, DRUM_KICK + DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED + DRUM_CLAP, 0, DRUM_HHOPEN, DRUM_KICK}
	,
		{DRUM_KICK + DRUM_HHCLOSED, 0, DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED, 0, DRUM_HHOPEN, DRUM_KICK,
		DRUM_HHCLOSED, 0, DRUM_KICK + DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED + DRUM_CLAP, 0, DRUM_HHOPEN, 0,

		DRUM_KICK + DRUM_HHCLOSED, 0, DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED, 0, DRUM_HHOPEN, DRUM_KICK,
		DRUM_HHCLOSED, 0, DRUM_KICK + DRUM_HHOPEN, 0,
		DRUM_SNARE + DRUM_HHCLOSED + DRUM_CLAP, 0, DRUM_SNARE + DRUM_HHOPEN, DRUM_SNARE + DRUM_KICK}
	,
		{DRUM_KICK + DRUM_CRASH, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0}
	,
		{DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK, 0, 0, 0,
		DRUM_KICK + DRUM_SNARE, 0, DRUM_SNARE, DRUM_SNARE}

	}
;



uint8_t songStep[SONG_STEP_NUMBER][SEQ_VOICES] = 
	{
		{1, 1, 0, 0, 1, 0, 0, 0},
		{1, 1, 0, 0, 1, 0, 0, 0},
		{1, 1, 0, 1, 1, 0, 0, 5},
		{1, 1, 0, 1, 1, 0, 0, 6},

		{1, 1, 1, 1, 1, 1, 1, 1},
		{2, 2, 2, 1, 1, 1, 1, 2},
		{1, 1, 1, 1, 1, 1, 1, 1},
		{2, 2, 2, 1, 1, 1, 1, 2},

		{3, 3, 3, 3, 3, 3, 3, 3},
		{4, 4, 4, 3, 3, 3, 3, 4},
		{3, 3, 3, 3, 3, 3, 3, 3},
		{4, 4, 4, 3, 3, 3, 3, 4},

		{1, 1, 1, 0, 0, 1, 0, 5},
		{2, 2, 2, 0, 0, 1, 0, 6},

		{1, 1, 1, 1, 1, 1, 1, 1},
		{2, 2, 2, 1, 1, 1, 1, 2},
		{1, 1, 1, 1, 1, 1, 1, 1},
		{2, 2, 2, 1, 1, 1, 1, 2},

		{3, 3, 3, 3, 3, 3, 3, 3},
		{4, 4, 4, 3, 3, 3, 3, 4},
		{3, 3, 3, 3, 3, 3, 3, 3},
		{4, 4, 4, 3, 3, 3, 3, 4},

		{1, 1, 1, 0, 1, 0, 0, 0},
		{1, 1, 0, 0, 1, 0, 0, 0}

	};
