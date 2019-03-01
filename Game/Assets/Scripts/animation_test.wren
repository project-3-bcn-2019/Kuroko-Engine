
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
ComponentType

import "Animation" for ComponentAnimation

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class animation_test is ObjectLinker{

construct new(){}

	animation { _animation }
	animation=(v) { _animation = v }

 Start() {
	
	_component_animation = getComponent(ComponentType.ANIMATION)
 }

 Update() {
	if(InputComunicator.getKey(InputComunicator.DOWN, InputComunicator.KEY_DOWN)){
		_component_animation.setAnimation(animation)
	}
	if(InputComunicator.getKey(InputComunicator.SPACE, InputComunicator.KEY_DOWN)){
		_component_animation.Play()
	}
	if(InputComunicator.getKey(InputComunicator.UP, InputComunicator.KEY_DOWN)){
		_component_animation.Pause()
	}
 }
}