class ParticleComunicator{
	foreign static C_CreateParticles(gameObject, component, particles)
}
class ComponentParticleEmitter{
	gameObject { _gameObject}		// UUID of the linked GO
	gameObject=(v){ _gameObject = v}

	component { _component}		// UUID of the component 
	component=(v){ _component = v}

	construct new(parent, uuid){
		gameObject = parent
		component = uuid
	}

	createParticles(particles){
		ParticleComunicator.C_CreateParticles(gameObject, component, particles)
	}

}