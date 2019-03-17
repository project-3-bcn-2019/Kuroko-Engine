
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
ComponentType,
Math

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class PlayerStats is ObjectLinker{

construct new(){}

    current_health {_health}
    current_health =(v){ _health = v}

    current_stamina {_stamina}
    current_stamina =(v){ _stamina = v}

    max_health {_max_health}
    max_health =(v){ _max_health = v}

    max_stamina {_max_stamina}
    max_stamina =(v){ _max_stamina = v}

    health_tag {_health_tag}
    health_tag =(v){ _health_tag = v}

    stamina_tag {_stamina_tag}
    stamina_tag =(v){ _stamina_tag = v}

 Start() {

    _hp_game_object = EngineComunicator.FindGameObjectsByTag(health_tag)[0]
    //_stamina_game_object = EngineComunicator.FindGameObjectsByTag(stamina_tag)[0]

    if(_hp_game_object == null) EngineComunicator.consoleOutput("hp null")

    _hp_progress_bar = _hp_game_object.getComponent(ComponentType.PROGRESS_BAR)
    //_stamina_progress_bar = _stamina_game_object.getComponent(ComponentType.PROGRESS_BAR)

 }

 Update() {

     this.UpdateHpBar()
     //this.UpdateStaminaBar()

    if(InputComunicator.getKey(InputComunicator.LEFT, InputComunicator.KEY_REPEAT)){
        current_health = current_health - 1
    }

    if(InputComunicator.getKey(InputComunicator.RIGHT, InputComunicator.KEY_REPEAT)){
        current_health = current_health + 1
    }

 }


 UpdateHpBar(){
    _hp_progress_bar.setProgress((current_health/max_health)*100)
 }

 UpdateStaminaBar(){
     _stamina_progress_bar.setProgress((current_stamina/max_stamina)*100)
 }

}