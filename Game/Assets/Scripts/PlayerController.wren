
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
Vec3,
Time

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class PlayerController is ObjectLinker{

    // Getters
    MovingState {__moving_state}
    IdleState {__idle_state}
    ShowDebugLogs{_show_debug_logs}


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

        //Initialize all the states as static so we have no problems switching to states at any moment
        __idle_state = IdleState.new(this)
        __punch1_state = BasicAttackState.new(this)
        __moving_state = MovingState.new(this)
        __dash_state = DashState.new(this)

        //this "this" I believe that should not be necesary but if removed, script won't compile    -p
        this.State = __idle_state //Reminder that "State" is a setter method
    }

    Update() {
        _player_state.HandleInput()
        _player_state.Update()
    }
}

class State {
    GetTotalDuration {_total_duration}
    GetCurrentTime {_current_time_in}

    construct new(player) {
        _player = player
    }

    //Called one time when switching to the state
    BeginState() {
        _current_time_in = 0

        if (_player.ShowDebugLogs) EngineComunicator.consoleOutput("new state began")
    }

    //Called each frame before update
    HandleInput() {

    }

    //Here are all the functions that all the states will do in update, remember to call super.Update() -p
    Update() {
        this.UpdateCurrentTime()
    }

    UpdateCurrentTime() {
        _current_time_in = _current_time_in + Time.C_GetDeltaTime() // += does not work -p

        if (_player.ShowDebugLogs)  EngineComunicator.consoleOutput("Current time in this state:%(_current_time_in)")
    }
}

class IdleState is State {
    construct new() {
    }

    construct new(player) {
        _player = player
        _direction = Vec3.zero()
        super(player)
    }

    BeginState() {
        super.BeginState()

        _direction = Vec3.zero()
    }

    HandleInput() {
        //We will use the direction just to check for input and switch to the moving state
        _direction.y = -InputComunicator.getAxisNormalized(-1,InputComunicator.L_AXIS_Y)
        _direction.x = InputComunicator.getAxisNormalized(-1,InputComunicator.L_AXIS_X)

        if(_direction.y < 0.1 && _direction.y > -0.1)   _direction.y = 0.0
        if(_direction.x < 0.1 && _direction.x > -0.1)   _direction.x = 0.0

        if(_direction.x != 0.0 || _direction.y != 0.0) _player.State = _player.MovingState
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
        _direction = Vec3.zero()
    }

    BeginState() {
        super.BeginState()

        _direction = Vec3.zero()
    }

    HandleInput() {
        //positive y means up and positive x means right
        _direction.y = -InputComunicator.getAxisNormalized(-1,InputComunicator.L_AXIS_Y)
        _direction.x = InputComunicator.getAxisNormalized(-1,InputComunicator.L_AXIS_X)

        if(_direction.y < 0.1 && _direction.y > -0.1)   _direction.y = 0.0
        if(_direction.x < 0.1 && _direction.x > -0.1)   _direction.x = 0.0

        if(_direction.x == 0.0 && _direction.y == 0.0) _player.State = _player.IdleState
    }
    
    Update() {
        super.Update()
        if (_player.ShowDebugLogs){  
            EngineComunicator.consoleOutput("Current state: Moving")
            EngineComunicator.consoleOutput("direction.x =%(_direction.x)")
            EngineComunicator.consoleOutput("direction.y =%(_direction.y)")
        }
    }
}

class DashState is State {
    construct new(player) {
        super(player)
        _player = player
    }

    HandleInput() {

    }
    
    Update() {
        EngineComunicator.consoleOutput("Dashing state")
    }
}

class AttackState is State {
    construct new(player) {
        super(player)
    }
}

class BasicAttackState is AttackState {
    construct new(player) {
        super(player)
        _player = player
    }

    HandleInput() {
        EngineComunicator.consoleOutput("Estoy handeleando el input que flipas como atake que soy")
    }
}

class SpecialAttackState is AttackState {
    construct new(player) {
        super(player)
    }
}