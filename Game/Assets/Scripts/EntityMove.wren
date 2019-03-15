
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

    acceleration {_acceleration}
    acceleration=(v){ _acceleration = v}

    max_speed {_max_speed}
    max_speed =(v){ _max_speed = v}


    //Rotation
    angular_acceleration {_angular_acceleration}
    angular_acceleration=(v){ _angular_acceleration = v}

    max_angular_speed {_max_angular_speed}
    max_angular_speed =(v){ _max_angular_speed = v}
    

    //Bool to check if you want the entity to always look forward
    face_movement {_face_movement}
    face_movement=(v){ _face_movement = v}


    Start() {
         _component_collider = getComponent(ComponentType.PHYSICS)

        //Vector to keep track of the actual velocity
         _speed_vector = Vec3.zero()
         _angular_speed = Vec3.zero()

        if (_component_collider == null){
            EngineComunicator.consoleOutput("Null collider")
        }

    }


    Update() {
      
        this.Break()

        //Update Speed
        if (_speed_vector.magnitude > 0){

            _component_collider.setSpeed(_speed_vector)
        }


        

        //Update rotation
        this.rotate(_angular_speed.x, _angular_speed.y, _angular_speed.z)

    }

    SetSpeed(x,y,z) {

        _speed_vector = Vec3.new(x,y,z)

    }


    SetSpeed(vec) {
        _speed_vector = vec

    }

    SetSpeedUnitary(x,y,z,speed){
        _speed_vector = Vec3.new(x,y,z)
        _speed_vector.normalize()
        _speed_vector*speed

    }

    AccelerateTo(x,y,z){

        //Update Speed 
        _speed_vector.x = _speed_vector.x + (acceleration * x * Time.C_GetDeltaTime())
        _speed_vector.y = _speed_vector.y + (acceleration * y * Time.C_GetDeltaTime())
        _speed_vector.z = _speed_vector.z + (acceleration * z * Time.C_GetDeltaTime())

        
        //Clamp 
        _speed_vector.x = Math.clamp(_speed_vector.x, -max_speed, max_speed)
        _speed_vector.y = Math.clamp(_speed_vector.y, -max_speed, max_speed)
        _speed_vector.z = Math.clamp(_speed_vector.z, -max_speed, max_speed)
    }

    AccelerateTo(vec){

        //Update Speed 
        _speed_vector.x = _speed_vector.x + (acceleration * vec.x * Time.C_GetDeltaTime())
        _speed_vector.y = _speed_vector.y + (acceleration * vec.y * Time.C_GetDeltaTime())
        _speed_vector.z = _speed_vector.z + (acceleration * vec.z * Time.C_GetDeltaTime())

        
        //Clamp 
        _speed_vector.x = Math.clamp(_speed_vector.x, -max_speed, max_speed)
        _speed_vector.y = Math.clamp(_speed_vector.y, -max_speed, max_speed)
        _speed_vector.z = Math.clamp(_speed_vector.z, -max_speed, max_speed)
    }

    Steer(vec){

        diff = Vec3.substract(vec, this.getPos("global"))
        diff = diff.normalized * acceleration

        this.AccelerateTo(diff)
    }

    Break() {
    //This function simulates the friction of the collider with the floor
    //It always slows down the speed

    _speed_vector.x = Math.lerp(_speed_vector.x, 0, _deceleration * Time.C_GetDeltaTime())
    _speed_vector.y = Math.lerp(_speed_vector.y, 0, _deceleration * Time.C_GetDeltaTime())
    _speed_vector.z = Math.lerp(_speed_vector.z, 0, _deceleration * Time.C_GetDeltaTime())
    
    }

    RotateTo(target){
        //Not tested

        //target_degrees = Math.cos
		//delta = Math.DeltaAngle(target_degrees, current_degrees);
        
        //var angle = Math.C_angleBetween(_speed_vector.x, _speed_vector.y, _speed_vector.z, target.x, target.y, target.z)

		//_angular_speed = _angular_speed + Math.clamp(angle, -max_angular_speed, max_angular_speed)

        _angular_speed.x = Math.lerp(_angular_speed.x, target.x, _angular_acceleration * Time.C_GetDeltaTime())
        _angular_speed.y = Math.lerp(_angular_speed.y, target.y, _angular_acceleration * Time.C_GetDeltaTime())
        _angular_speed.z = Math.lerp(_angular_speed.z, target.z, _angular_acceleration * Time.C_GetDeltaTime())

    }

    RotateTo(x,y,z){
        
        var angle = Math.C_angleBetween(_angular_speed.x, _angular_speed.y, _angular_speed.z, x, y, z)


        //_angular_speed.x = Math.lerp(_speed_vector.x, x, _angular_acceleration * Time.C_GetDeltaTime())
        //_angular_speed.y = Math.lerp(_speed_vector.y, y, _angular_acceleration)
        //_angular_speed.z = Math.lerp(_speed_vector.z, z, _angular_acceleration * Time.C_GetDeltaTime())

        EngineComunicator.consoleOutput("X:%(_angular_speed.x)")
        EngineComunicator.consoleOutput("Y:%(_angular_speed.y)")
        EngineComunicator.consoleOutput("Z:%(_angular_speed.z)")

    }


    SetRotationVelocity(dir){

        _angular_speed = Math.clamp(dir, -max_angular_speed, max_angular_speed)

    }

  
}