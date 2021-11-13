#pragma once

namespace m2d
{
namespace MIDIBLE
{
	namespace MIDI
	{
		enum ControlChangeNumber : uint8_t
		{
			// High resolution Continuous Controllers MSB (+32 for LSB) ----------------
			BankSelect = 0,
			ModulationWheel = 1,
			BreathController = 2,
			// CC3 undefined
			FootController = 4,
			PortamentoTime = 5,
			DataEntry = 6,
			ChannelVolume = 7,
			Balance = 8,
			// CC9 undefined
			Pan = 10,
			ExpressionController = 11,
			EffectControl1 = 12,
			EffectControl2 = 13,
			// CC14 undefined
			// CC15 undefined
			GeneralPurposeController1 = 16,
			GeneralPurposeController2 = 17,
			GeneralPurposeController3 = 18,
			GeneralPurposeController4 = 19,

			// Switches ----------------------------------------------------------------
			Sustain = 64,
			Portamento = 65,
			Sostenuto = 66,
			SoftPedal = 67,
			Legato = 68,
			Hold = 69,

			// Low resolution continuous controllers -----------------------------------
			SoundController1 = 70, ///< Synth: Sound Variation   FX: Exciter On/Off
			SoundController2 = 71, ///< Synth: Harmonic Content  FX: Compressor On/Off
			SoundController3 = 72, ///< Synth: Release Time      FX: Distortion On/Off
			SoundController4 = 73, ///< Synth: Attack Time       FX: EQ On/Off
			SoundController5 = 74, ///< Synth: Brightness        FX: Expander On/Off
			SoundController6 = 75, ///< Synth: Decay Time        FX: Reverb On/Off
			SoundController7 = 76, ///< Synth: Vibrato Rate      FX: Delay On/Off
			SoundController8 = 77, ///< Synth: Vibrato Depth     FX: Pitch Transpose On/Off
			SoundController9 = 78, ///< Synth: Vibrato Delay     FX: Flange/Chorus On/Off
			SoundController10 = 79, ///< Synth: Undefined         FX: Special Effects On/Off
			GeneralPurposeController5 = 80,
			GeneralPurposeController6 = 81,
			GeneralPurposeController7 = 82,
			GeneralPurposeController8 = 83,
			PortamentoControl = 84,
			// CC85 to CC90 undefined
			Effects1 = 91, ///< Reverb send level
			Effects2 = 92, ///< Tremolo depth
			Effects3 = 93, ///< Chorus send level
			Effects4 = 94, ///< Celeste depth
			Effects5 = 95, ///< Phaser depth

			// Channel Mode messages ---------------------------------------------------
			AllSoundOff = 120,
			ResetAllControllers = 121,
			LocalControl = 122,
			AllNotesOff = 123,
			OmniModeOff = 124,
			OmniModeOn = 125,
			MonoModeOn = 126,
			PolyModeOn = 127
		};
	}
}
}