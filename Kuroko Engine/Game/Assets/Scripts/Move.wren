import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator

class Move is ObjectLinker{
	construct new(){}

	speed { __speed }
	speed=(v) { __speed = v }
    
    Start() {}

	Update(){
		if(InputComunicator.getKey(InputComunicator.UP, InputComunicator.KEY_REPEAT)){
			modPos(0,0,speed)
		}
		
		if(InputComunicator.getKey(InputComunicator.DOWN, InputComunicator.KEY_REPEAT)){
			modPos(0,0,-speed)
		}

		if(InputComunicator.getKey(InputComunicator.LEFT, InputComunicator.KEY_REPEAT)){
			modPos(speed,0,0)
		}
		
		if(InputComunicator.getKey(InputComunicator.RIGHT, InputComunicator.KEY_REPEAT)){
			modPos(-speed,0,0)
		}
	}
}



