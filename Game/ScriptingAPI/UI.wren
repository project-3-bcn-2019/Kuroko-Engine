class ButtonComunicator{
	foreign static C_ButtonGetState(gameObject, component)

	static IDLE {0}
	static MOUSE_OVER {1}
	static PRESSED {2}
}
class ComponentButton{
	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	component { _component}		// UUID of the component 
	component=(v){ _component = v}

	construct new(parent, uuid){
		gameObject = parent
		component = uuid
	}

	getState(){
		return ButtonComunicator.C_ButtonGetState(gameObject, component)
	}
}

class CheckboxComunicator{
	foreign static C_CheckboxIsPressed(gameObject, component)
}
class ComponentCheckbox{
	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	component { _component}		// UUID of the component 
	component=(v){ _component = v}

	construct new(parent, uuid){
		gameObject = parent
		component = uuid
	}

	isPressed(){
		return CheckboxComunicator.C_CheckboxIsPressed(gameObject, component)
	}
}

class TextComunicator{
	foreign static C_SetText(gameObject, component, text)
}

class ComponentText{
	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	component { _component}		// UUID of the component 
	component=(v){ _component = v}

	
	construct new(parent, uuid){
		gameObject = parent
		component = uuid
	}

	setText(text){
		return TextComunicator.C_SetText(gameObject, component, text)
	}
}

class ProgressBarComunicator{
	foreign static C_SetProgress(gameObject, component, progress)
}

class ComponentProgressBar {
	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	component { _component}		// UUID of the component 
	component=(v){ _component = v}

	
	construct new(parent, uuid){
		gameObject = parent
		component = uuid
	}

	setProgress(progress){
		return ProgressBarComunicator.C_SetProgress(gameObject, component, progress)
	}
}