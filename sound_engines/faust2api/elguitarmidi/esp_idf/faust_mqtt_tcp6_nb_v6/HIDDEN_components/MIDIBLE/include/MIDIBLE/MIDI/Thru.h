#pragma once

namespace m2d
{
namespace MIDIBLE
{
	namespace MIDI
	{
		/*! Enumeration of Thru filter modes */
		struct Thru
		{
			enum Mode
			{
				Off = 0, ///< Thru disabled (nothing passes through).
				Full = 1, ///< Fully enabled Thru (every incoming message is sent back).
				SameChannel = 2, ///< Only the messages on the Input Channel will be sent back.
				DifferentChannel = 3, ///< All the messages but the ones on the Input Channel will be sent back.
			};
		};
	}
}
}