
class AnimationComunicator{
	foreign static C_SetAnimation(gameObject, component, sound)
	foreign static C_Play(gameObject, component)
	foreign static C_Pause(gameObject, component)
}
class ComponentAnimation{
	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	component { _component}		// UUID of the component 
	component=(v){ _component = v}

	construct new( parent, uuid){
		gameObject = parent
		component = uuid
	}

	setAnimation(animation){
		AnimationComunicator.C_SetAnimation(gameObject, component, animation)
	}

	Play(){
		AnimationComunicator.C_Play(gameObject, component)
	}
	Pause(){
		AnimationComunicator.C_Pause(gameObject, component)
	}

}