
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
ComponentType

import "Animation" for ComponentAnimator

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class animator_test is ObjectLinker{

construct new(){}

 Start() {
	_animator = getComponent(ComponentType.ANIMATOR)
 }

 Update() {
	if(InputComunicator.getKey(InputComunicator.DOWN, InputComunicator.KEY_DOWN)){
		_animator.setBool("Boolean", false)
	}
	if(_animator.getBool("Boolean")){
        EngineComunicator.consoleOutput("Boolean is true")
    }
    if(InputComunicator.getKey(InputComunicator.SPACE, InputComunicator.KEY_DOWN)){
		_animator.setString("Text", "SomethingELSE")
	}
    if(InputComunicator.getKey(InputComunicator.UP, InputComunicator.KEY_DOWN)){
        EngineComunicator.consoleOutput(_animator.getString("Text"))
	}

	if(InputComunicator.getKey(InputComunicator.LEFT, InputComunicator.KEY_DOWN)){
        _animator.setInt("Int", 6)
	}
	if(InputComunicator.getKey(InputComunicator.RIGHT, InputComunicator.KEY_DOWN)){
		var number = _animator.getInt("Int")
        EngineComunicator.consoleOutput("%(number)")
	}
  }
}