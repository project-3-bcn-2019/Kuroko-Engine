
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

class PlayerController is ObjectLinker{

    // Getters
    ShowDebugLogs{_show_debug_logs}

    // Below this the values are not meant to be changed in the inspector (also there is no setter so they can't)
    MovingState {_moving_state}
    IdleState {_idle_state}
    DashState {_dash_state}
    Punch1 {_punch1_state}
    MoveDirection {_move_direction}
    OldMoveDirection {_old_move_direction}

    ComponentAnimation {_component_animation}
    ComponentAudioSource {_component_audio_source}

    Speed {_speed}

    PunchButton {_punch_button}
    KickButton {_kick_button}
    DashButton {_dash_button}


    // Setters
    State = (new_state) {
        _player_state = new_state   
        _player_state.BeginState()
    }

    ShowDebugLogs = (value) {_show_debug_logs = value} 


    construct new(){}

    Start() {
        _show_debug_logs = true
        _player_state
        _move_direction = Vec3.zero()
        _old_move_direction = Vec3.zero()
        _speed = 0.5

        _punch_button = InputComunicator.C_X
        _kick_button = InputComunicator.C_Y
        _dash_button = InputComunicator.C_A

        //Components
        _component_animation = getComponent(ComponentType.ANIMATION)
        _component_audio_source = getComponent(ComponentType.AUDIO_SOURCE)

        _component_audio_source.setSound("Footsteps") //This should not be here -Pol

        //Initialize all the states
        //the arguments are: (player, total_duration)
        _idle_state = IdleState.new(this)
        _punch1_state = BasicAttackState.new(this,700)
        _moving_state = MovingState.new(this)
        _dash_state = DashState.new(this,500)


        //this "this" I believe that should not be necesary but if removed, script won't compile    -p
        this.State = _idle_state //Reminder that "State" is a setter method
    }

    Update() {
        this.CalculateDirection()
        _player_state.HandleInput()
        _player_state.Update()
    }

    CalculateDirection() {
        //positive y means up and positive x means right
        _move_direction.y = -InputComunicator.getAxisNormalized(-1,InputComunicator.L_AXIS_Y)
        _move_direction.x = -InputComunicator.getAxisNormalized(-1,InputComunicator.L_AXIS_X)

        if(InputComunicator.getKey(InputComunicator.UP, InputComunicator.KEY_REPEAT)){
	       _move_direction.y = 1
        }
	 	
        if(InputComunicator.getKey(InputComunicator.DOWN, InputComunicator.KEY_REPEAT)){
            _move_direction.y = -1
        }

        if(InputComunicator.getKey(InputComunicator.LEFT, InputComunicator.KEY_REPEAT)){
            _move_direction.x = 1
        }
                
        if(InputComunicator.getKey(InputComunicator.RIGHT, InputComunicator.KEY_REPEAT)){
        _move_direction.x = -1
        }

        if(_move_direction.y < 0.2 && _move_direction.y > -0.2)   _move_direction.y = 0.0
        if(_move_direction.x < 0.2 && _move_direction.x > -0.2)   _move_direction.x = 0.0
    }

    modPos(x,y,z) {
        super.modPos(x,y,z)
    } 

    rotate(x,y,z) {
        super.rotate(x,y,z)
    }

    CheckBoundaries(movement){
        //Collision stuff delete later
        var pos_x = getPosX("global")
        var pos_z = getPosZ("global")

        if (pos_x > 40){
            if (movement.x > 0){
                movement.x = 0
            }
        }
        if (pos_x < -40 ){
            if (movement.x < 0){
                movement.x = 0
            }
        }

        if (pos_z > 40 ){
            if (movement.z > 0){
                movement.z = 0
            }
        }
        if (pos_z < -40 ){
            if (movement.z < 0){
                movement.z = 0
            }
        }
    }
}

class State {
    TotalDuration {_total_duration}
    CurrentTime {_current_time_in}

    construct new(player) {
        _player = player
    }

    construct new(player,total_duration) {
        _player = player
        _total_duration = total_duration
    }

    //Called one time when switching to the state
    BeginState() {
        _current_time_in = 0

        if (_player.ShowDebugLogs) EngineComunicator.consoleOutput("new state began")
    }

    //Here are all the functions that all the states will do in update, remember to call super.Update() -p
    Update() {
        this.UpdateCurrentTime()
    }

    UpdateCurrentTime() {
        _current_time_in = _current_time_in + Time.C_GetDeltaTime() // += does not work -p

        if (_player.ShowDebugLogs)  EngineComunicator.consoleOutput("Current time in this state:%(_current_time_in)")
    }

    IsStateFinished() {
        var ret = false
        if (_current_time_in >= _total_duration) ret = true
        return ret
    }
}

class IdleState is State {
    construct new() {
    }

    construct new(player) {
        _player = player
        super(player)
    }

    BeginState() {
        _player.ComponentAnimation.setAnimation("PunchingAnimation")
        _player.ComponentAnimation.Reset()
        _player.ComponentAnimation.Pause()
        super.BeginState()
    }

    HandleInput() {
        // If l-stick is not still switch to moving
        if(_player.MoveDirection.x != 0.0 || _player.MoveDirection.y != 0.0) _player.State = _player.MovingState
        // If X prassed switch to punch
        if (InputComunicator.getButton(-1,_player.PunchButton, InputComunicator.KEY_DOWN)) _player.State = _player.Punch1
        if (InputComunicator.getKey(InputComunicator.J, InputComunicator.KEY_DOWN)) _player.State = _player.Punch1
    }

    Update() {
        super.Update()
        if (_player.ShowDebugLogs){  
            EngineComunicator.consoleOutput("Current state: Idle")
        }
    }

}

class MovingState is State {

    construct new(player) {
        super(player)
        _player = player
    }

    BeginState() {
        super.BeginState()
        _player.ComponentAnimation.setAnimation("RunningAnimation")
        _player.ComponentAnimation.Reset()
        _player.ComponentAnimation.Play()
    }

    HandleInput() {
        if(_player.MoveDirection.x == 0.0 && _player.MoveDirection.y == 0.0) _player.State = _player.IdleState
        // If A prassed switch to dash
        if (InputComunicator.getButton(-1,_player.DashButton, InputComunicator.KEY_DOWN)) _player.State = _player.DashState
        if (InputComunicator.getKey(InputComunicator.SPACE, InputComunicator.KEY_DOWN)) _player.State = _player.DashState
        // If X prassed switch to dash
        if (InputComunicator.getButton(-1,_player.PunchButton, InputComunicator.KEY_DOWN)) _player.State = _player.Punch1
        if (InputComunicator.getKey(InputComunicator.J, InputComunicator.KEY_DOWN)) _player.State = _player.Punch1
    }
    
    Update() {
        super.Update()

        var movement = Vec3.new(_player.MoveDirection.x*_player.Speed,0,_player.MoveDirection.y*_player.Speed)

        _player.CheckBoundaries(movement)

        _player.modPos(movement.x,movement.y,movement.z)

        var angle = Math.C_angleBetween(_player.OldMoveDirection.x,_player.OldMoveDirection.y,_player.OldMoveDirection.z,_player.MoveDirection.x,_player.MoveDirection.y,_player.MoveDirection.z)
        _player.rotate(_player.MoveDirection.x,_player.MoveDirection.y,_player.MoveDirection.z)
        //_player.rotate(_player.MoveDirection.x, _player.MoveDirection.y, _player.MoveDirection.z)
        _player.OldMoveDirection.x = _player.MoveDirection.x
        _player.OldMoveDirection.y = _player.MoveDirection.y
        _player.OldMoveDirection.z = _player.MoveDirection.z

        if (_player.ShowDebugLogs){  
            EngineComunicator.consoleOutput("Current state: Moving")
            EngineComunicator.consoleOutput("direction.x =%(_player.MoveDirection.x)")
            EngineComunicator.consoleOutput("direction.y =%(_player.MoveDirection.y)")
        }
    }
}

class DashState is State {
    construct new(player) {
        super(player)
        _player = player
    }

    construct new(player,total_duration) {
        _player = player
        super(player,total_duration)
    }

    BeginState() {
        super.BeginState()
        _player.ComponentAnimation.setAnimation("DashingAnimation")
        _player.ComponentAnimation.Reset()
        _player.ComponentAnimation.Play()
        _dash_direction = Vec3.new(_player.MoveDirection.x,_player.MoveDirection.y,_player.MoveDirection.z)
        _dash_speed = 1
    }

    HandleInput() {

    }
    
    Update() {
        super.Update() 

        var movement = Vec3.new(_dash_direction.x*_dash_speed,0,_dash_direction.y*_dash_speed)

        _player.CheckBoundaries(movement)
        _player.modPos(movement.x,movement.y,movement.z)

        if (super.IsStateFinished()) _player.State = _player.IdleState

        if (_player.ShowDebugLogs){  
            EngineComunicator.consoleOutput("direction.x =%(_dash_direction.x)")
            EngineComunicator.consoleOutput("direction.y =%(_dash_direction.y)")
            EngineComunicator.consoleOutput("Current state: Dash")
        }
    }
}

class AttackState is State {
    construct new(player) {
        super(player)
    }

    construct new(player,total_duration) {
        super(player,total_duration)
    }
}

class BasicAttackState is AttackState {
    construct new(player) {
        super(player)
        _player = player
    }

    construct new(player,total_duration) {
        _player = player
        super(player,total_duration)
    }

    BeginState() {
        super.BeginState()
        _player.ComponentAnimation.setAnimation("PunchingAnimation")
        _player.ComponentAnimation.Reset()
        _player.ComponentAnimation.Play()
        _player.ComponentAudioSource.setSound("Punch")
        _player.ComponentAudioSource.Play()
        _player.ComponentAudioSource.setSound("Footsteps")
    }

    HandleInput() {
    
    }

    Update() {
        super.Update() 

        if (super.IsStateFinished()) _player.State = _player.IdleState

        EngineComunicator.consoleOutput("Current state: BasicAttack")
    }
}

class SpecialAttackState is AttackState {
    construct new(player) {
        super(player)
    }
}




