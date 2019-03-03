
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
    ShowDebugLogs{_show_debug_logs}

    // Below this the values are not meant to be changed in the inspector (also there is no setter so they can't)
    MovingState {__moving_state}
    IdleState {__idle_state}
    DashState {__dash_state}
    Punch1 {__punch1_state}
    MoveDirection {_move_direction}


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

        //Initialize all the states as static so we have no problems switching to states at any moment
        //the arguments are: (player, total_duration)
        __idle_state = IdleState.new(this)
        __punch1_state = BasicAttackState.new(this,3000)
        __moving_state = MovingState.new(this)
        __dash_state = DashState.new(this,2000)

        //this "this" I believe that should not be necesary but if removed, script won't compile    -p
        this.State = __idle_state //Reminder that "State" is a setter method
    }

    Update() {
        this.CalculateDirection()
        _player_state.HandleInput()
        _player_state.Update()
    }

    CalculateDirection() {
        //positive y means up and positive x means right
        _move_direction.y = -InputComunicator.getAxisNormalized(-1,InputComunicator.L_AXIS_Y)
        _move_direction.x = InputComunicator.getAxisNormalized(-1,InputComunicator.L_AXIS_X)

        if(_move_direction.y < 0.1 && _move_direction.y > -0.1)   _move_direction.y = 0.0
        if(_move_direction.x < 0.1 && _move_direction.x > -0.1)   _move_direction.x = 0.0
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
        super.BeginState()
    }

    HandleInput() {
        // If l-stick is not still switch to moving
        if(_player.MoveDirection.x != 0.0 || _player.MoveDirection.y != 0.0) _player.State = _player.MovingState
        // If A prassed switch to dash
        if (InputComunicator.getButton(0,InputComunicator.C_A, InputComunicator.KEY_DOWN)) _player.State = _player.DashState
        // If X prassed switch to dash
        if (InputComunicator.getButton(0,InputComunicator.C_X, InputComunicator.KEY_DOWN)) _player.State = _player.Punch1
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
    }

    HandleInput() {
        if(_player.MoveDirection.x == 0.0 && _player.MoveDirection.y == 0.0) _player.State = _player.IdleState
        // If A prassed switch to dash
        if (InputComunicator.getButton(0,InputComunicator.C_A, InputComunicator.KEY_DOWN)) _player.State = _player.DashState
        // If X prassed switch to dash
        if (InputComunicator.getButton(0,InputComunicator.C_X, InputComunicator.KEY_DOWN)) _player.State = _player.Punch1
    }
    
    Update() {
        super.Update()
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

    HandleInput() {

    }
    
    Update() {
        super.Update() 

        if (super.IsStateFinished()) _player.State = _player.IdleState

        EngineComunicator.consoleOutput("Current state: Dash")
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