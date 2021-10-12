idf_component_register(SRCS "src/jdksmidi_midi.cpp" "src/jdksmidi_parser.cpp" "src/jdksmidi_msg.cpp" "src/jdksmidi_sysex.cpp" 
		       INCLUDE_DIRS "include/jdksmidi" "include"                      
                       REQUIRES 			# Edit following two lines to set component requirements (see docs)
                       PRIV_REQUIRES )

