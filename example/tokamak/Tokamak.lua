--! / bin / lua

TWOPI=3.141592653589793*2.0
N_PHI=100
Context = {

    CoordinateSystem={
        Cartesian = {
            Type = "Cartesian",
            x0 = { 0, 0, 0 },
        },
        Cylindrical = {
            Type = "Cylindrical",
            x0 = { 0, 0, 0 },
        },
    },

    Mesh = {
        Default = {
            Type = "SMesh",
            CoordinateSystem="Cylindrical",
            Scale = { 0.1, 0.1, TWOPI/N_PHI },
            PeriodicDimension = { 0, 0, 1 },
        },
    },
    Domains =
    {
        Tokamak = {
            Type = "EMFluid",
            Mesh = "Default",
            Model = "Tokamak",
            BoundaryCondition = {
                Type = "PEC",
                Model = "Tokamak.Limiter",
            },
            InitialCondition = {
                ne = "Tokamak.ne",
                B0v = "Tokamak.B0",
            },
            Species = {
                ele = { Z = -1.0, mass = 1.0 / 1836, ratio = 1.0 },
                H = { Z = 1.0, mass = 1.0, ratio = 1.0 },
            }
        },
        RFAntenna = {
            Type = "ExtraSource",
            Mesh = "Default",
            Variable = "E",
            IsHard = false,
            Model = "Antenna",
            Amplify = { 0.0, 0.0, 1.0 },
            WaveNumber = 0.0,
            Frequency = 1.0e9
        },
    },
    Model = {
        Tokamak = {
            Type = "GEqdsk",
            gfile = "/home/salmon/workspace/SimPla/scripts/gfile/g038300.03900",
            Phi = { -3.14 / 8, 3.14 / 8 },
        },
        Antenna = {
            Type = "Cube",
            lo = { 2.2, -0.1, -3.1415926 / 64 },
            hi = { 2.25, 0.1, 3.1415926 / 64 }
        },
    },
}

Schedule = {
    Type = "SAMRAITimeIntegrator",
    OutputURL = "TokamakSaveData",
    TimeBegin = 0.0,
    TimeEnd = 5e-9,
    TimeStep = 1.0e-11,
    CheckPointInterval = 10,
    UpdateOrder = { "RFAntenna", "Tokamak" }
}