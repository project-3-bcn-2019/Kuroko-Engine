
class PhysicsComunicator{
	foreign static C_SetSpeed(gameObject, component, x, y, z)
}
class ComponentPhysics{
	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	component { _component}		// UUID of the component 
	component=(v){ _component = v}

	construct new(parent, uuid){
		gameObject = parent
		component = uuid
	}

	setSpeed(speed){
		PhysicsComunicator.C_SetSpeed(gameObject, component, speed.x, speed.y, speed.z)
	}

}