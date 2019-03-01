
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
ComponentType

import "Audio" for ComponentAudioSource

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class sound_test is ObjectLinker{

	sound_name { _sound_name }
	sound_name=(v) { _sound_name = v }

	audio_source {_audio_source}
	audio_source=(v) {_audio_source = v}

	construct new(){}

	Start() {
		 audio_source = getComponent(ComponentType.AUDIO_SOURCE)
         audio_source.setSound(sound_name)
	}

	Update() {
		if(InputComunicator.getButton(-1, InputComunicator.C_A, InputComunicator.KEY_DOWN)){
			audio_source.Play()
		}
	}
}