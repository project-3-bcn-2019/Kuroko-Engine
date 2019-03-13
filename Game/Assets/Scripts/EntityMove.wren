
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
Vec3,
Time,
ComponentType,
Math

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class EntityMove is ObjectLinker{

construct new(){}

    //Put deceleration like 0.01 or will turn crazy (Lerp doesn't work very well)

    deceleration {_deceleration}
    deceleration=(v){ _deceleration = v}

    Start() {
         _component_collider = getComponent(ComponentType.PHYSICS)

        //Vector to keep track of the actual velocity
         _speed_vector = Vec3.zero()


        if (_component_collider == null){
            EngineComunicator.consoleOutput("Null collider")
        }

    }


    Update() {

        this.Break()
    }

    SetSpeed(x,y,z) {

        _speed_vector = Vec3.new(x,y,z)
        _component_collider.setSpeed(_speed_vector)

    }

    SetSpeedUnitary(x,y,z,speed){
        _speed_vector = Vec3.new(x,y,z)
        _speed_vector.normalize()
        _speed_vector*speed
        EngineComunicator.consoleOutput("%(_speed_vector.x)")
        EngineComunicator.consoleOutput("%(_speed_vector.y)")
        EngineComunicator.consoleOutput("%(_speed_vector.z)")
        _component_collider.setSpeed(_speed_vector)
    }


    Break() {
    //This function simulates the friction of the collider with the floor
    //It always slows down the speed

    _speed_vector.x = Math.lerp(_speed_vector.x, 0, _deceleration * Time.C_GetDeltaTime())
    _speed_vector.y = Math.lerp(_speed_vector.y, 0, _deceleration * Time.C_GetDeltaTime())
    _speed_vector.z = Math.lerp(_speed_vector.z, 0, _deceleration * Time.C_GetDeltaTime())

    _component_collider.setSpeed(_speed_vector)
    
    }

    SetDeceleration(value) {
        _deceleration = value
    }
  
}