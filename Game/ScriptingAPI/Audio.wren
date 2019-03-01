
class AudioSourceComunicator{
	foreign static C_SetSound(gameObject, component, sound)
	foreign static C_Play(gameObject, component)
	foreign static C_Pause(gameObject, component)
	foreign static C_Resume(gameObject, component)
	foreign static C_Stop(gameObject, component)
}
class ComponentAudioSource{
	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	component { _component}		// UUID of the component 
	component=(v){ _component = v}

	construct new( parent, uuid){
		gameObject = parent
		component = uuid
	}

	setSound(sound){
		AudioSourceComunicator.C_SetSound(gameObject, component, sound)
	}

	Play(){
		AudioSourceComunicator.C_Play(gameObject, component)
	}
	Pause(){
		AudioSourceComunicator.C_Pause(gameObject, component)
	}
	Resume(){
		AudioSourceComunicator.C_Resume(gameObject, component)
	}
	Stop(){
		AudioSourceComunicator.C_Stop(gameObject, component)
	}

}