Group {
    id: box
    label: "Envelope"
    property Function whenModified: nil

    function cb()
    {
        whenModified.call if whenModified
    }

    ParModuleRow {
        id: top
        Knob {
            whenValue: lambda { box.cb };
            extern: box.extern+"A_dt"
            type:   :float
        }
        Knob {
            whenValue: lambda { box.cb };
            extern: box.extern+"D_dt"
            type:   :float
        }
        Knob {
            whenValue: lambda { box.cb };
            extern: box.extern+"PS_val"
        }
        Knob {
            whenValue: lambda { box.cb };
            extern: box.extern+"R_dt"
            type:   :float
        }
    }
    ParModuleRow {
        id: bot
        Knob     { whenValue: lambda { box.cb }; extern: box.extern+"Penvstretch"}
        ToggleButton   { label: "lin/log"; whenValue: lambda { box.cb }; extern: box.extern+"Plinearenvelope"}
        Col {
            ToggleButton   { label: "FRCR"; whenValue: lambda { box.cb }; extern: box.extern+"Pforcedrelease"}
            ToggleButton   { label: "repeat"; whenValue: lambda { box.cb }; extern: box.extern+"Prepeating"}
        }
    }
}
