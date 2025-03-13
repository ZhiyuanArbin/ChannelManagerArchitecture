flowchart TD
    %% Main method
    Start[Start runCCCV] --> Subscribe[Subscribe to channel data]
    Subscribe --> AddCCTask[Add CCTask to queue]
    AddCCTask --> RegisterVoltageCallback[Register voltage callback]
    RegisterVoltageCallback --> RegisterLimitCallback[Register step limit callback]
    RegisterLimitCallback --> End[End runCCCV method]
    
    %% Worker Thread Processing
    subgraph "Worker Thread"
        WT_Start[Worker Thread Loop] --> WT_Wait{Wait for task or stop signal}
        WT_Wait -->|Task available| WT_GetTask[Get task from queue]
        WT_GetTask --> WT_Execute[Execute task]
        WT_Execute --> WT_CheckType{Task type?}
        WT_CheckType -->|CCTask| WT_CC[Execute Constant Current]
        WT_CheckType -->|CVTask| WT_CV[Execute Constant Voltage]
        WT_CheckType -->|CallbackControlTask| WT_Callback[Execute Callback]
        WT_CheckType -->|Other Task| WT_Other[Execute Other Task]
        WT_CC --> WT_Delete[Delete task]
        WT_CV --> WT_Delete
        WT_Callback --> WT_Delete
        WT_Other --> WT_Delete
        WT_Delete --> WT_Start
        WT_Wait -->|Stop signal| WT_Exit[Exit thread]
    end
    
    %% M4 Data Thread
    subgraph "M4 Data Thread"
        M4_Start[M4 Data Thread Loop] --> M4_Read[Read data from M4]
        M4_Read --> M4_Process[Process data for all channels]
        M4_Process --> M4_AddDataTasks[Add data processing tasks]
        M4_AddDataTasks --> M4_CheckSub{Channel subscribed?}
        M4_CheckSub -->|Yes| M4_Handle[Handle callbacks]
        M4_CheckSub -->|No| M4_Sleep[Sleep briefly]
        M4_Handle --> M4_Sleep
        M4_Sleep --> M4_Start
    end
    
    %% Callback Processing
    subgraph "Callback Execution"
        CB_Start[Callback Control Task] --> CB_GetData[Get channel data]
        CB_GetData --> CB_Check{Check conditions}
        CB_Check -->|Voltage reached| CB_AddCV[Add CV Task]
        CB_AddCV --> CB_Unreg[Unregister voltage callback]
        CB_Unreg --> CB_RegNew[Register new callback]
        CB_Check -->|Limit reached| CB_End[End test]
        CB_End --> CB_UnregAll[Unregister all callbacks]
        CB_UnregAll --> CB_Unsub[Unsubscribe channel]
        CB_Unsub --> CB_Term[Terminate test]
    end
    
    %% Thread Interactions
    End -.-> WT_Start
    M4_Handle -.-> AddCallbackTask[Add CallbackControlTask to queue]
    AddCallbackTask -.-> WT_Wait
    CB_AddCV -.-> AddCVTask[Add CV Task to queue]
    AddCVTask -.-> WT_Wait
    
    %% Task Flow
    AddCCTask -.-> TaskQueue((Task Queue))
    AddCallbackTask -.-> TaskQueue
    AddCVTask -.-> TaskQueue
    TaskQueue -.-> WT_GetTask
    
    %% Data Flow
    M4_Read -.-> DataFlow((Channel Data))
    DataFlow -.-> CB_GetData
    
    %% Legend
    classDef mainThread fill:#f9d5e5,stroke:#333,stroke-width:1px
    classDef workerThread fill:#eeeeee,stroke:#333,stroke-width:1px
    classDef m4Thread fill:#d5f9e5,stroke:#333,stroke-width:1px
    classDef callbackExec fill:#e5d5f9,stroke:#333,stroke-width:1px
    classDef dataFlow fill:#f9e5d5,stroke:#333,stroke-width:1px
    
    class Start,Subscribe,AddCCTask,RegisterVoltageCallback,RegisterLimitCallback,End mainThread
    class WT_Start,WT_Wait,WT_GetTask,WT_Execute,WT_CheckType,WT_CC,WT_CV,WT_Callback,WT_Other,WT_Delete,WT_Exit workerThread
    class M4_Start,M4_Read,M4_Process,M4_AddDataTasks,M4_CheckSub,M4_Handle,M4_Sleep m4Thread
    class CB_Start,CB_GetData,CB_Check,CB_AddCV,CB_Unreg,CB_RegNew,CB_End,CB_UnregAll,CB_Unsub,CB_Term callbackExec
    class TaskQueue,DataFlow,AddCallbackTask,AddCVTask dataFlow