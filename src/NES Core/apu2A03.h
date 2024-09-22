#pragma once
#include <cstdint>
#include <memory>
#include <fstream>

class apu2A03
{
public:
	apu2A03();
	~apu2A03();

public:
	void cpuWrite(uint16_t addr, uint8_t data);
	uint8_t cpuRead(uint16_t addr);
	void clock();
	void reset();

	void toggleMute();
	void setVolume(uint8_t volume);
	void setPulse1Volume(uint8_t volume);
	void setPulse2Volume(uint8_t volume);
	void setTriangleVolume(uint8_t volume);
	void setNoiseVolume(uint8_t volume);
	void setDmcVolume(uint8_t volume);

	uint8_t DMC_sample_byte = 0;
	bool DMC_DMA_load = false;
	bool DMC_DMA_reload = false;
	bool DMC_DMA_alignment = false;
	bool DMC_DMA_dummy = false;
	bool DMC_DMA_done = false;
	bool buffer_filled = false;
	bool IRQ = false;

	std::unique_ptr<uint8_t[]> audio_buffer = std::make_unique<uint8_t[]>(BUFFER_SIZE);

private:
	bool mute = false;
	float master_volume = 1.0f;
	float pulse1_volume = 1.0f;
	float pulse2_volume = 1.0f;
	float triangle_volume = 1.0f;
	float noise_volume = 1.0f;
	float DMC_volume = 1.0f;
	double output = 0.0;

	static const uint16_t BUFFER_SIZE = 512;
	uint16_t buffer_index = 0;

	uint32_t clock_counter = 0;
	uint32_t pulse_hz = 0;
	bool four_step_sequence_mode = true;
	bool quarter_frame_clock = false;
	bool half_frame_clock = false;

	double pulse_out = 0.0;
	double tnd_out = 0.0;
	double pulse_table[31];
	double tnd_table[203];

	// Duty sequences
	static constexpr uint8_t duty_sequences[4][8] =
	{
		{ 0, 1, 0, 0, 0, 0, 0, 0},
		{ 0, 1 ,1 ,0 ,0, 0, 0, 0},
		{ 0, 1, 1, 1, 1, 0, 0, 0},
		{ 1, 0, 0, 1, 1, 1, 1, 1}
	};

	// Length counter lookup table
	static constexpr uint8_t length_counter_lookup[32] =
	{ 10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
	12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30 };

	// Triangle 32-step sequence
	static constexpr uint8_t triangle_sequence[32] =
	{ 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

	// Noise channel period lookup table
	static constexpr uint16_t noise_period_lookup[16] =
	{ 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 };

	// DMC rate lookup table
	static constexpr uint16_t DMC_rate_lookup[16] =
	{ 428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54 };

	// Sound channel components
	struct sequencer
	{
		uint8_t duty_cycle = 0x00;
		uint8_t cycle_position = 0x00;
		uint16_t timer = 0x0000;
		uint16_t reload = 0x0000;
		uint8_t output = 0x00;
	};
	struct linear_counter
	{
		bool control = false;
		bool reload_flag = false;
		uint8_t counter = 0x00;
		uint8_t reload = 0x00;
	};
	struct envelope
	{
		bool start_flag = false;
		bool loop = false;
		bool constant_volume = true;
		uint8_t volume = 0x00;
		uint8_t timer = 0x00;
		uint8_t output = 0x00;
		uint8_t decay_level_counter = 0x00;
	};
	struct sweep
	{
		bool enable = false;
		bool negate = false;
		bool reload_flag = false;
		bool mute = false;
		uint8_t pulse_channel_number = 0;
		uint8_t shift_count = 0x00;
		int16_t change = 0x0000;
		uint16_t timer = 0x0000;
		uint16_t reload = 0x0000;
		int16_t target_period = 0x0000;
	};
	struct length_counter
	{
		bool enable = false;
		bool halt = false;
		uint8_t timer = 0x00;
	};
	struct memoryReader
	{
		uint16_t address = 0x0000;
		int16_t remaining_bytes = 0;
	};
	struct outputUnit
	{
		uint8_t shift_register = 0x00;
		int16_t remaining_bits = 0;
		uint8_t output_level = 0;
		bool silence_flag = false;
	};

	// Sound Channels 
	struct pulseChannel
	{
		sequencer seq;
		envelope env;
		sweep sweep;
		length_counter len_counter;
	};
	struct triangleChannel
	{
		sequencer seq;
		length_counter len_counter;
		linear_counter linear_counter;
	};
	struct noiseChannel
	{
		envelope env;
		length_counter len_counter;
		uint16_t timer = 0x0000;
		uint16_t reload = 0x0000;
		uint16_t shift_register = 0x01;
		uint8_t output = 0x00;
		bool mode = false;
	};
	struct DMCChannel
	{
		bool IRQ_flag = false;
		bool loop_flag = false;
		bool sample_buffer_empty = false;
		uint8_t sample_buffer = 0x00;
		uint16_t sample_address = 0x0000;
		uint16_t sample_length = 0x0000;
		uint16_t timer = 0x0000;
		uint16_t reload = 0x0000;
		outputUnit output_unit;
		memoryReader memory_reader;
	};

	bool interrupt_inhibit = false;
	// Channel 1 - Pulse 1
	pulseChannel pulse1;
	bool pulse1_enable = false;

	// Channel 2 - Pulse 2
	pulseChannel pulse2;
	bool pulse2_enable = false;

	// Channel 3 - Triangle
	triangleChannel triangle;
	bool triangle_enable = false;

	// Channel 4 - Noise
	noiseChannel noise;
	bool noise_enable = false;

public: 
	// Channel 5 - DMC
	DMCChannel DMC;
	bool DMC_enable = false;

	void generateSample();

	void pulseChannelClock(sequencer& seq, bool enable);
	void triangleChannelClock(triangleChannel& triangle, bool enable);
	void noiseChannelClock(noiseChannel& noise, bool enable);
	void DMCChannelClock(DMCChannel& DMC, bool enable);

	uint16_t getDMCAddress();
	void setDMCBuffer(uint8_t value);

	void soundChannelEnvelopeClock(envelope& envelope);
	void soundChannelSweeperClock(pulseChannel& channel);

	void soundChannelLengthCounterClock(length_counter& len_counter);
	void linearCounterClock(linear_counter& linear_counter);

};

