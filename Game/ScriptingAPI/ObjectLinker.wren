import "Audio" for ComponentAudioSource
import "Animation" for ComponentAnimation
import "Particles" for ComponentParticleEmitter

class ObjectComunicator{
	foreign static C_setPos(gameObject, x, y, z)
	foreign static C_modPos(gameObject, x, y, z)
        foreign static C_rotate(gameObject, x, y, z)
	foreign static C_lookAt(gameObject, x, y, z)

	foreign static C_getPosX(gameObject, mode)
	foreign static C_getPosY(gameObject, mode)
	foreign static C_getPosZ(gameObject, mode)

	foreign static C_getPitch(gameObject)
	foreign static C_getYaw(gameObject)
	foreign static C_getRoll(gameObject)

	foreign static C_Kill(gameObject)
	foreign static C_MoveForward(gameObject, speed)

	foreign static C_GetComponentUUID(gameObject, component_type)
	foreign static C_GetCollisions(gameObject)

}

class Math{
	foreign static C_sqrt(number)

	static pow(number, power){
		var ret = 1
		for(i in 0...power){
			ret = ret * number
		}
		return ret
	}

	static lerp(a, b, f){
		return a + (f * (b - a))
	}

	static clamp(value, min, max){
		if (value < min){
			value = min
		}
		if (value > max){
			value = max
		}

		return value
	}

      foreign static C_angleBetween(x_1,y_1,z_1,x_2,y_2,z_2)


}

class Time{
	foreign static C_GetDeltaTime()
	foreign static C_GetTimeScale()
	foreign static C_SetTimeScale(multiplier)

}

class Vec3{

	construct zero(){
		x = 0
		y = 0
		z = 0
	}

	construct new(n_x, n_y, n_z){
		x = n_x
		y = n_y
		z = n_z
	}

	x {_x}
	x=(v) {_x = v}

	y {_y}
	y=(v) {_y = v}

	z {_z}
	z=(v) {_z = v}

	magnitude {Math.C_sqrt(Math.pow(x, 2) + Math.pow(y, 2) + Math.pow(z, 2))}
	
	normalized { Vec3.new(x/magnitude, y/magnitude, z/magnitude)}

	normalize(){
		var static_magnitude = magnitude
		x = x/static_magnitude
		y = y/static_magnitude
		z = z/static_magnitude
	}

	static add(vec1, vec2){
		return ret = Vec3.new(vec1.x + vec2.x,vec1.y + vec2.y,vec1.z + vec2.z)
	}
	
	copy(vec){
		x = vec.x
		y = vec.y
		z = vec.z
	}

}

class EngineComunicator{
	// Foreigners User usable
	foreign static consoleOutput(message)
	foreign static getTime()
	foreign static BreakPoint(message, variable, variable_name)
	foreign static LoadScene(scene_name)

	// Foreigners Intermediate
	foreign static C_FindGameObjectsByTag(tag)
	foreign static C_Instantiate(prefab_name, x, y, z, pitch, yaw, roll)


	// Static User usable
	static Instantiate(prefab_name, pos, euler){
		EngineComunicator.C_Instantiate(prefab_name, pos.x, pos.y, pos.z, euler.x, euler.y, euler.z)
	}

	static FindGameObjectsByTag(tag){
		var uuids = EngineComunicator.C_FindGameObjectsByTag(tag)
		var gameObjects = []
		for(i in 0...uuids.count){
			var add_obj = ObjectLinker.new()
			add_obj.gameObject = uuids[i]
			gameObjects.insert(i, add_obj)
		}

		return gameObjects
	}
}

class InputComunicator{
	
	static UP {82}
	static DOWN {81}
	static LEFT {80}
	static RIGHT {79}
	static SPACE {44}

	static D_PAD_UP {11}
	static D_PAD_DOWN {12}
	static D_PAD_LEFT {13}
	static D_PAD_RIGHT {14}

	static L_AXIS_UP {18}
	static L_AXIS_DOWN {17}
	static L_AXIS_LEFT {15}
	static L_AXIS_RIGHT {16}

	static L_AXIS_X {0}
	
	static L_AXIS_Y {1}

	static C_A {0}
	static C_B {1}
	static C_X {2}
	static C_Y {3}



	static KEY_DOWN {1}
	static KEY_REPEAT {2}
	static KEY_UP {3}

	foreign static getKey(key, mode)

	foreign static getButton(controller_id, button, mode)
	foreign static getAxis(controller_id, axis)

	foreign static getMouseRaycastX()
	foreign static getMouseRaycastY()
	foreign static getMouseRaycastZ()

	static getMouseRaycast(){
		return Vec3.new(InputComunicator.getMouseRaycastX(), InputComunicator.getMouseRaycastY(), InputComunicator.getMouseRaycastZ())
	}
	
	static getAxisNormalized(controller_id, axis){
		return getAxis(controller_id, axis)/32767
	}
}


class ComponentType{
	static AUDIO_SOURCE {15}
	static ANIMATION {7}
	static PARTICLES {18}
}

class ObjectLinker{
	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	construct new(){}

	setPos(x,y,z){
		ObjectComunicator.C_setPos(gameObject, x, y, z)
	}
	modPos(x,y,z){
		ObjectComunicator.C_modPos(gameObject, x, y, z)
	}
        rotate(x,y,z){
                ObjectComunicator.C_rotate(gameObject, x, y, z)
        }
	lookAt(x,y,z){
		ObjectComunicator.C_lookAt(gameObject, x, y, z)
	}

	kill(){
	    ObjectComunicator.C_Kill(gameObject)
	}

	getPos(mode){
		return Vec3.new(getPosX(mode), getPosY(mode), getPosZ(mode))
	}

	getPosX(mode){
		return ObjectComunicator.C_getPosX(gameObject, mode)
	}

	getPosY(mode){
		return ObjectComunicator.C_getPosY(gameObject, mode)
	}

	getPosZ(mode){
		return ObjectComunicator.C_getPosZ(gameObject, mode)
	}

	getEuler(){
		return Vec3.new(getPitch(), getYaw(), getRoll())
	}

	getPitch(){
		return ObjectComunicator.C_getPitch(gameObject)
	}

	getYaw(){
		return ObjectComunicator.C_getYaw(gameObject)
	}

	getRoll(){
		return ObjectComunicator.C_getRoll(gameObject)
	}

	moveForward(speed){
		ObjectComunicator.C_MoveForward(gameObject, speed)
	}

	getCollisions(){
		var uuids = ObjectComunicator.C_GetCollisions(gameObject)
		var gameObjects = []
		for(i in 0...uuids.count){
			var add_obj = ObjectLinker.new()
			add_obj.gameObject = uuids[i]
			gameObjects.insert(i, add_obj)
		}

		return gameObjects
	}

	// Returns a class depending on the component
	getComponent(type){
		var component_uuid = ObjectComunicator.C_GetComponentUUID(gameObject, type)

		if(type == ComponentType.AUDIO_SOURCE){
			return ComponentAudioSource.new(gameObject, component_uuid)
		}
		if(type == ComponentType.ANIMATION){
			return ComponentAnimation.new(gameObject, component_uuid)
		}
		if(type == ComponentType.PARTICLES){
			return ComponentParticleEmitter.new(gameObject, component_uuid)
		}

	}
}