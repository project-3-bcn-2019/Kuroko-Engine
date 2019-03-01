import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator

class Move is ObjectLinker{
	construct new(){}

	speed { __speed }
	speed=(v) { __speed = v }
    
    Start() {}

	Update(){
		
		// Up with keyboard
		if(InputComunicator.getKey(InputComunicator.UP, InputComunicator.KEY_REPEAT)){
			modPos(0,0,speed)
		}

		// Up with axis value
		if(InputComunicator.getAxis(-1, InputComunicator.L_AXIS_Y) < -30000){
			modPos(0,0,speed)
		}
		// Up with axis bool
		if(InputComunicator.getButton(-1, InputComunicator.L_AXIS_UP, InputComunicator.KEY_REPEAT)){
			modPos(0,0,speed)
		}
	}
}



