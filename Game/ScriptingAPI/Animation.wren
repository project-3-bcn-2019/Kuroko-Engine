
class AnimationComunicator{

	// Component Animation
	foreign static C_SetAnimation(gameObject, component, animation)
	foreign static C_Play(gameObject, component)
	foreign static C_Pause(gameObject, component)
    foreign static C_ResetAnimation(gameObject, component)

	// Component Animator
	foreign static C_SetInt(gameObject, component, name, value)
	foreign static C_GetInt(gameObject, component, name)
	foreign static C_SetFloat(gameObject, component, name, value)
	foreign static C_GetFloat(gameObject, component, name)
	foreign static C_SetString(gameObject, component, name, value)
    foreign static C_GetString(gameObject, component, name)
	foreign static C_SetBool(gameObject, component, name, value)
    foreign static C_GetBool(gameObject, component, name)
	foreign static C_SetSpeed(gameObejct, component, speed)


}
class ComponentAnimation{
	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	component { _component}		// UUID of the component 
	component=(v){ _component = v}

	construct new(parent, uuid){
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
    Reset(){
         AnimationComunicator.C_ResetAnimation(gameObject, component)
    }
}

class ComponentAnimator{

	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	component { _component}		// UUID of the component 
	component=(v){ _component = v}

	construct new(parent, uuid){
		gameObject = parent
		component = uuid
	}

	setInt(name, value){
		AnimationComunicator.C_SetInt(gameObject, component, name, value)
	}
	getInt(name){
		return AnimationComunicator.C_GetInt(gameObject, component, name)
	}
	setFloat(name, value){
		AnimationComunicator.C_SetFloat(gameObject, component, name, value)
	}
	getFloat(name){
		return AnimationComunicator.C_GetFloat(gameObject, component, name)
	}
	setString(name, value){
		AnimationComunicator.C_SetString(gameObject, component, name, value)
	}
	getString(name){
		return AnimationComunicator.C_GetString(gameObject, component, name)
	}
	setBool(name, value){
		AnimationComunicator.C_SetBool(gameObject, component, name, value)
	}
	getBool(name){
		return AnimationComunicator.C_GetBool(gameObject, component, name)
	}
	setSpeed(speed){
		AnimationComunicator.C_SetSpeed(gameObject, component, speed)
	}

}