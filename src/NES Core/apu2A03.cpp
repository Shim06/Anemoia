#include "apu2A03.h"
#include <iostream>

apu2A03::apu2A03()
{
	// Initialize pulse table and tnd table
	for (int n = 0; n < 31; n++)
	{
		pulse_table[n] = 95.52 / (8128.0 / n + 100);
	}

	for (int n = 0; n < 203; n++)
	{
		tnd_table[n] = 163.67 / (24329.0 / n + 100);
	}

	pulse1.sweep.pulse_channel_number = 1;
	pulse2.sweep.pulse_channel_number = 2;
}

apu2A03::~apu2A03()
{
}

void apu2A03::cpuWrite(uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x4000:
		pulse1.seq.duty_cycle = ((data & 0xC0) >> 6);

		pulse1.env.loop = ((data >> 5) & 0x01);
		pulse1.len_counter.halt = ((data >> 5) & 0x01);
		pulse1.env.constant_volume = ((data >> 4) & 0x01);
		pulse1.env.volume = (data & 0x0F);
		break;

	case 0x4001:
		pulse1.sweep.enable = (data >> 7);
		pulse1.sweep.reload = ((data >> 4) & 0x07) + 1;
		pulse1.sweep.negate = ((data >> 3) & 0x01);
		pulse1.sweep.shift_count = data & 0x07;
		pulse1.sweep.reload_flag = true;
		break;

	case 0x4002:
		pulse1.seq.reload = (pulse1.seq.reload & 0xFF00) | data;
		break;

	case 0x4003:
		pulse1.seq.cycle_position = 0;
		pulse1.seq.reload = (pulse1.seq.reload & 0x00FF) | (uint16_t)((data & 0x07) << 8);
		pulse1.seq.timer = pulse1.seq.reload;
		pulse1.env.start_flag = true;	

		if (pulse1_enable) pulse1.len_counter.timer = length_counter_lookup[data >> 3] + 1;

		// Restart envelope
		pulse1.env.timer = pulse1.env.volume;
		pulse1.env.decay_level_counter = 15;
		break;

	case 0x4004:
		pulse2.seq.duty_cycle = ((data & 0xC0) >> 6);

		pulse2.env.loop = ((data >> 5) & 0x01);
		pulse2.len_counter.halt = pulse2.env.loop;
		pulse2.env.constant_volume = ((data >> 4) & 0x01);
		pulse2.env.volume = (data & 0x0F);
		break;

	case 0x4005:
		pulse2.sweep.enable = (data >> 7);
		pulse2.sweep.reload = ((data >> 4) & 0x07) + 1;
		pulse2.sweep.negate = ((data >> 3) & 0x01);
		pulse2.sweep.shift_count = data & 0x07;
		pulse2.sweep.reload_flag = true;
		break;

	case 0x4006:
		pulse2.seq.reload = (pulse2.seq.reload & 0xFF00) | data;
		break;

	case 0x4007:
		pulse2.seq.cycle_position = 0;
		pulse2.seq.reload = (pulse2.seq.reload & 0x00FF) | (uint16_t)((data & 0x07) << 8);
		pulse2.seq.timer = pulse2.seq.reload;
		pulse2.env.start_flag = true;

		if (pulse2_enable) pulse2.len_counter.timer = length_counter_lookup[data >> 3] + 1;

		// Restart envelope
		pulse2.env.timer = pulse2.env.volume;
		pulse2.env.decay_level_counter = 15;
		break;

	case 0x4008:
		triangle.linear_counter.reload = data & 0x7F;
		triangle.len_counter.halt = data >> 7;
		triangle.linear_counter.control = data >> 7;
		break;

	case 0x400A:
		triangle.seq.reload = (triangle.seq.reload & 0xFF00) | data;
		break;

	case 0x400B:
		triangle.seq.reload = ((triangle.seq.reload & 0x00FF) | (uint16_t)((data & 0x07)) << 8) + 1;
		triangle.seq.timer = triangle.seq.reload;

		if (triangle_enable) triangle.len_counter.timer = length_counter_lookup[data >> 3] + 1;
		triangle.linear_counter.reload_flag = true;
		break;

	case 0x400C:
		noise.len_counter.halt = (data >> 5) & 0x01;
		noise.env.constant_volume = (data >> 4) & 0x01;
		noise.env.volume = data & 0x0F;
		break;

	case 0x400E:
		noise.mode = data >> 7;
		noise.reload = noise_period_lookup[data & 0x0F] / 2;
		break;

	case 0x400F:
		noise.env.start_flag = true;

		if (noise_enable) noise.len_counter.timer = length_counter_lookup[data >> 3] + 1;
		break;

	case 0x4010:
		DMC.IRQ_flag = data >> 7;
		DMC.loop_flag = (data & 0x40) == 0x40;
		DMC.reload = (DMC_rate_lookup[data & 0x0F] / 2) - 1;
		DMC.timer = DMC.reload;
		break;

	case 0x4011:
		DMC.output_unit.output_level = data & 0x7F;
		break;

	case 0x4012:
		DMC.sample_address = 0xC000 | ((uint32_t)data << 6);
		DMC.memory_reader.address = DMC.sample_address;
		break;

	case 0x4013:
		DMC.sample_length = (data << 4) | 0x0001;
		DMC.memory_reader.remaining_bytes = DMC.sample_length;
		break;

	case 0x4015:
		IRQ = false;
		// Pulse 1 enable
		if (data & 0x01)
		{
			pulse1_enable = true;
		}
		else
		{
			pulse1_enable = false;
			pulse1.len_counter.timer = 0;
		}

		// Pulse 2 enable
		if ((data >> 1) & 0x01)
		{
			pulse2_enable = true;
		}
		else
		{
			pulse2_enable = false;
			pulse2.len_counter.timer = 0;
		}

		// Triangle enable
		if ((data >> 2) & 0x01)
		{
			triangle_enable = true;
		}
		else
		{
			triangle_enable = false;
			triangle.len_counter.timer = 0;
		}

		// Noise enable
		if ((data >> 3) & 0x01)
		{
			noise_enable = true;
		}
		else
		{
			noise_enable = false;
			noise.len_counter.timer = 0;
		}

		// DMC enable
		if ((data >> 4) & 0x01)
		{
			DMC_enable = true;
			if (DMC.sample_buffer_empty)
			{
				DMC_DMA_load = true;
				DMC_DMA_alignment = true;
				DMC_DMA_dummy = true;
			}
		}
		else
		{
			DMC_enable = false;
		}
		break;

	case 0x4017:
		if ((data >> 7) == 0) four_step_sequence_mode = true;
		else four_step_sequence_mode = false;

		if (((data >> 6) & 0x01) == 1)
		{
			IRQ = false; 
			interrupt_inhibit = true;
		}
		else interrupt_inhibit = false;
		break;
	}
}

uint8_t apu2A03::cpuRead(uint16_t addr)
{
	uint8_t data = 0x00;
	if (addr == 0x4015)
	{
		IRQ = false;
		data = 0x00;
	}
	return data;
}

void apu2A03::clock()
{
	// Clock all sound channels
	if (clock_counter % 2 == 0)
	{
		pulseChannelClock(pulse1.seq, pulse1_enable);
		pulseChannelClock(pulse2.seq, pulse2_enable);
		noiseChannelClock(noise, noise_enable);
		DMCChannelClock(DMC, DMC_enable);
	}
	triangleChannelClock(triangle, triangle_enable);

	quarter_frame_clock = false;
	half_frame_clock = false;
	if (four_step_sequence_mode)
	{
		if (clock_counter == 7457)
		{
			quarter_frame_clock = true;
		}
		else if (clock_counter == 14913)
		{
			quarter_frame_clock = true;
			half_frame_clock = true;
		}
		else if (clock_counter == 22371)
		{
			quarter_frame_clock = true;
		}
		else if (clock_counter == 29829)
		{
			if (!interrupt_inhibit) IRQ = true;
			quarter_frame_clock = true;
			half_frame_clock = true;
			clock_counter = 1;
		}
	}
	else
	{
		if (clock_counter == 7457)
		{
			quarter_frame_clock = true;
		}
		else if (clock_counter == 14913)
		{
			quarter_frame_clock = true;
			half_frame_clock = true;
		}
		else if (clock_counter == 22371)
		{
			quarter_frame_clock = true;
		}
		else if (clock_counter == 37281)
		{
			quarter_frame_clock = true;
			half_frame_clock = true;
			clock_counter = 1;
		}
	}

	// Clock envelope and triangle linear counter
	if (quarter_frame_clock)
	{
		soundChannelEnvelopeClock(pulse1.env);
		soundChannelEnvelopeClock(pulse2.env);
		soundChannelEnvelopeClock(noise.env);
		linearCounterClock(triangle.linear_counter);
	}

	// Clock sweeper and length counter
	if (half_frame_clock)
	{
		soundChannelSweeperClock(pulse1);
		soundChannelLengthCounterClock(pulse1.len_counter);

		soundChannelSweeperClock(pulse2);
		soundChannelLengthCounterClock(pulse2.len_counter);

		soundChannelLengthCounterClock(triangle.len_counter);
		soundChannelLengthCounterClock(noise.len_counter);
	}

	// Mute sound channels if muted
	if (pulse1.sweep.mute || pulse1.seq.reload < 8 || pulse1.len_counter.timer == 0)
	{
		pulse1.seq.output = 0;
		pulse1.env.output = 0;
	}
	if (pulse2.sweep.mute || pulse2.seq.reload < 8 || pulse2.len_counter.timer == 0)
	{
		pulse2.seq.output = 0;
		pulse2.env.output = 0;
	}
	// Silencing the triangle channel when triangle.seq.reload < 2 is considered less accurate emulation,
	// but eliminates high frequencies and popping
	//if (!triangle_enable || triangle.len_counter.timer == 0 || triangle.seq.reload < 2) 
	//{
	//	triangle.seq.output = 0;
	//}

	// Put sound channels output into audio buffers
	// Generate sample evey 40.584421768 clocks
	// 1.789773 MHz / 44100 Hz
	if (pulse_hz > 1789773 * 2)
	{
		generateSample();
		pulse_hz -= 1789773;
	}

	pulse_out = pulse_table[(pulse1.seq.output ? pulse1.env.output : 0)
						  + (pulse2.env.output ? pulse2.env.output : 0)];

	tnd_out = tnd_table[3 * triangle.seq.output
					  + 2 * ((!(noise.shift_register & 0x01) && noise.len_counter.timer > 0) ? noise.env.output : 0)
					  + DMC.output_unit.output_level];
	output = pulse_out + tnd_out;

	pulse_hz += 44100;
	clock_counter++;
}

void apu2A03::reset()
{
	pulse1_enable = false;
	pulse2_enable = false;
	triangle_enable = false;
	noise_enable = false;
	DMC_enable = false;
	IRQ = false;

	pulse1.len_counter.timer = 0;
	pulse2.len_counter.timer = 0;
	triangle.len_counter.timer = 0;
	noise.len_counter.timer = 0;

	DMC.output_unit.output_level = 0;
	DMC.output_unit.remaining_bits = 0;
	DMC.output_unit.shift_register = 0;
	DMC.memory_reader.address = 0;
	DMC.memory_reader.remaining_bytes = 0;
	DMC.timer = 0;
	DMC.sample_address = 0;
	DMC.sample_buffer = 0;
	DMC.sample_length = 0;
	DMC.output_unit.silence_flag = true;
}

void apu2A03::toggleMute()
{
	mute = !mute;
}

void apu2A03::setVolume(uint8_t volume)
{
	master_volume = volume / 100.0f;
}

void apu2A03::setPulse1Volume(uint8_t volume)
{
	pulse1_volume = volume / 100.0f;
}

void apu2A03::setPulse2Volume(uint8_t volume)
{
	pulse2_volume = volume / 100.0f;
}

void apu2A03::setTriangleVolume(uint8_t volume)
{
	triangle_volume = volume / 100.0f;
}

void apu2A03::setNoiseVolume(uint8_t volume)
{
	noise_volume = volume / 100.0f;
}

void apu2A03::setDmcVolume(uint8_t volume)
{
	DMC_volume = volume / 100.0f;
}

void apu2A03::generateSample()
{
	audio_buffer[buffer_index] = 0;
	audio_buffer[buffer_index] += static_cast<int>(std::round((pulse1.seq.output ? pulse1.env.output : 0) * pulse1_volume));
	audio_buffer[buffer_index] += static_cast<int>(std::round(pulse2.seq.output ? pulse2.env.output: 0) * pulse2_volume);
	audio_buffer[buffer_index] += static_cast<int>(std::round(triangle.seq.output * triangle_volume));
	audio_buffer[buffer_index] += static_cast<int>(std::round(DMC.output_unit.output_level * DMC_volume));
	if (!(noise.shift_register & 0x01) && noise.len_counter.timer > 0)
		audio_buffer[buffer_index] += static_cast<int>(std::round(noise.env.output * noise_volume));

	audio_buffer[buffer_index] = static_cast<int>(std::round(audio_buffer[buffer_index] * master_volume));

	if (mute) audio_buffer[buffer_index] = 1;
	if (audio_buffer[buffer_index] == 0) audio_buffer[buffer_index] = 1;
	//audio_buffer[buffer_index] = static_cast<uint8_t>(audio_buffer[buffer_index] * output);
	buffer_index++;

	// Reset audio buffer index once filled
	if (buffer_index >= BUFFER_SIZE)
	{
		buffer_index = 0;
		buffer_filled = true;
	}
}

void apu2A03::pulseChannelClock(sequencer& seq, bool enable)
{
	if (!enable)
		return;

	seq.timer--;
	if (seq.timer == 0xFFFF)
	{
		seq.timer = seq.reload;
		// Shift duty cycle with wrapping
		seq.output = duty_sequences[seq.duty_cycle][seq.cycle_position];
		seq.cycle_position++;
		if (seq.cycle_position >= 8) seq.cycle_position = 0;
	}
}

void apu2A03::triangleChannelClock(triangleChannel& triangle, bool enable)
{
	triangle.seq.timer--;
	if (triangle.seq.timer == 0)
	{
		triangle.seq.timer = triangle.seq.reload;
		if (!(triangle.len_counter.timer > 0 && triangle.linear_counter.counter > 0))
			return;

		if (triangle.seq.reload >= 2)
		{
			triangle.seq.output = triangle_sequence[triangle.seq.duty_cycle];
			//std::cout << +triangle.seq.output << "\n";
			//std::cout << +triangle.seq.duty_cycle << "\n\n";
			triangle.seq.duty_cycle++;
			if (triangle.seq.duty_cycle >= 32) triangle.seq.duty_cycle = 0;
		}
	}
}

void apu2A03::noiseChannelClock(noiseChannel& noise, bool enable)
{
	noise.timer--;
	if (noise.timer == 0xFFFF)
	{
		noise.timer = noise.reload;
		uint8_t temp = noise.mode ? (noise.shift_register >> 6) & 0x01 : (noise.shift_register >> 1) & 0x01;
		noise.output = (noise.shift_register & 0x01) ^ (temp);
		noise.shift_register >>= 1;
		noise.shift_register |= noise.output << 14;
	}
}

void apu2A03::DMCChannelClock(DMCChannel& DMC, bool enable)
{
	if (!enable)
		return;

	DMC.timer--;
	if (DMC.timer == 0xFFFF)
	{
		DMC.timer = DMC.reload + 1;
		if (DMC.output_unit.silence_flag == false)
		{
			if (DMC.output_unit.shift_register & 0x01)
			{
				if (DMC.output_unit.output_level <= 125)
					DMC.output_unit.output_level += 2;
			}
			else
			{
				if (DMC.output_unit.output_level >= 2)
					DMC.output_unit.output_level -= 2;
			}

			DMC.output_unit.shift_register >>= 1;
		}

		// Update Bits remaining counter
		DMC.output_unit.remaining_bits--;
		if (DMC.output_unit.remaining_bits <= 0)
		{
			DMC.output_unit.remaining_bits = 8;

			if (DMC.sample_buffer_empty)
			{
				DMC.output_unit.silence_flag = true;
			}
			else
			{
				DMC.output_unit.silence_flag = false;
				DMC.output_unit.shift_register = DMC.sample_buffer;
				DMC.sample_buffer_empty = true;
				DMC_DMA_reload = true;
				DMC_DMA_alignment = true;
				DMC_DMA_dummy = true;
			}
		}
	}
}

void apu2A03::setDMCBuffer(uint8_t value)
{
	if (DMC.memory_reader.remaining_bytes > 0)
	{
		DMC.sample_buffer = value;
		DMC.sample_buffer_empty = false;

		DMC.memory_reader.address++;
		if (DMC.memory_reader.address == 0x0000)
			DMC.memory_reader.address = 0x8000;

		DMC.memory_reader.remaining_bytes--;
		if (DMC.memory_reader.remaining_bytes == 0)
		{
			if (DMC.loop_flag)
			{
				// Restart sample
				DMC.memory_reader.address = DMC.sample_address;
				DMC.memory_reader.remaining_bytes = DMC.sample_length;
			}
			else if (DMC.IRQ_flag) IRQ = true;
		}
	}
}

uint16_t apu2A03::getDMCAddress()
{
	return DMC.memory_reader.address;
}

void apu2A03::soundChannelEnvelopeClock(envelope& envelope)
{
	if (envelope.start_flag)
	{
		envelope.start_flag = false;
		envelope.decay_level_counter = 15;
		envelope.timer = envelope.volume + 1;
	}
	else
	{
		envelope.timer--;
		if (envelope.timer == 0)
		{
			envelope.timer = envelope.volume + 1;
			if (envelope.decay_level_counter > 0) envelope.decay_level_counter--;
			else if (envelope.loop) envelope.decay_level_counter = 15;
		}
	}

	if (envelope.constant_volume) envelope.output = envelope.volume;
	else envelope.output = envelope.decay_level_counter;
}

void apu2A03::soundChannelSweeperClock(pulseChannel& channel)
{
	// Calculate the target period
	channel.sweep.change = (channel.seq.reload >> channel.sweep.shift_count);
	// Negate change if negate flag is true
	// Pulse 1 adds one's complement = -c - 1
	// Pulse 2 adds two's complement = -c
	if (channel.sweep.negate && channel.sweep.pulse_channel_number == 1)
		channel.sweep.change = -channel.sweep.change - 1;
	else if (channel.sweep.negate && channel.sweep.pulse_channel_number == 2)
		channel.sweep.change = -channel.sweep.change;

	channel.sweep.target_period = channel.seq.reload + channel.sweep.change;
	if (channel.sweep.target_period < 0)
		channel.sweep.target_period = 0;

	// Check if channel should be muted
	if (channel.seq.reload < 8) channel.sweep.mute = true;
	else if (channel.sweep.target_period > 0x7FF) channel.sweep.mute = true;
	else channel.sweep.mute = false;

	channel.sweep.timer--;
	if (channel.sweep.enable && channel.sweep.timer == 0 && channel.sweep.shift_count != 0)
	{
		if (!channel.sweep.mute)
			channel.seq.reload = channel.sweep.target_period;
	}
	if (channel.sweep.timer == 0 || channel.sweep.reload_flag)
	{
		channel.sweep.timer = channel.sweep.reload;
		channel.sweep.reload_flag = false;
	}
}

void apu2A03::soundChannelLengthCounterClock(length_counter& len_counter)
{
	if (len_counter.timer > 0 && !len_counter.halt)
		len_counter.timer--;
}

void apu2A03::linearCounterClock(linear_counter& linear_counter)
{
	if (linear_counter.reload_flag)
		linear_counter.counter = linear_counter.reload;
	else if (linear_counter.counter > 0)
			linear_counter.counter--;

	if (linear_counter.control == 0)
		linear_counter.reload_flag = false;
}
