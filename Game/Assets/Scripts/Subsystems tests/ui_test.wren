
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
ComponentType

import "UI" for ComponentCheckbox, ComponentButton, ButtonComunicator, ComponentText

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class ui_test is ObjectLinker{

construct new(){}

 Start() {
	//_checkbox = getComponent(ComponentType.CHECK_BOX)
	//_button = getComponent(ComponentType.BUTTON)
	//_text = getComponent(ComponentType.TEXT)
	_progress_bar = getComponent(ComponentType.PROGRESS_BAR)
 }

 Update() {
	//if(_checkbox.isPressed()){
	//	EngineComunicator.consoleOutput("hi")
	//}

	//if(_button.getState() == ButtonComunicator.IDLE){
	//	EngineComunicator.consoleOutput("button_idle")
	//}

	//if(_button.getState() == ButtonComunicator.MOUSE_OVER){
	//	EngineComunicator.consoleOutput("mouse over")
	//}

	//if(_button.getState() == ButtonComunicator.PRESSED){
	//	EngineComunicator.consoleOutput("pressed")
	//}

	//if(InputComunicator.getKey(InputComunicator.UP, InputComunicator.KEY_DOWN)){
	//	_text.setText("UP")
	//}
	//if(InputComunicator.getKey(InputComunicator.DOWN, InputComunicator.KEY_DOWN)){
	//	_text.setText("DOWN")
	//}

	if(InputComunicator.getKey(InputComunicator.UP, InputComunicator.KEY_DOWN)){
		_progress_bar.setProgress(50)
	}
	if(InputComunicator.getKey(InputComunicator.DOWN, InputComunicator.KEY_DOWN)){
		_progress_bar.setProgress(25)
	}
 }
}