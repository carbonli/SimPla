Description="For Cold Plasma Dispersion" -- description or other text things.    
-- SI Unit System
c = 299792458  -- m/s
KeV = 1.1604e7    -- K
Tesla = 1.0       -- Tesla    
--

Btor= 1.2  * Tesla
Ti =  0.03 * KeV
Te =  0.05 * KeV

rhoi = 4.57*1e-3 * math.sqrt(Ti)/Btor  --1.02 * math.sqrt(Ti)/(1e4*Btor) -- m
--    print(rhoi)

k0 = 25./40.
NX = 101
NY = 1
NZ = 1
LX = 100 --0.6
LY = 0 --2.0*math.pi/k0
LZ = 0  -- 2.0*math.pi/18
GW = 5 
N0 = 2.4e18 -- 4*Btor*Btor* 5.327934360e15 -- m^-3

omega_ci = 9.578309e7 * Btor -- e/m_p B0 rad/s

-- From Gan

InitN0=function(x,y,z)
      local X0 = 12*LX/NX;
      local DEN_JUMP = 0.4*LX;
      local DEN_GRAD = 0.2*LX;
      local AtX0 = 2./math.pi*math.atan((-DEN_JUMP)/DEN_GRAD);
      local AtLX = 2./math.pi*math.atan((LX-DEN_JUMP-X0)/DEN_GRAD);
      local DenCof = 1./(AtLX-AtX0);
      local dens1 = DenCof*(2./math.pi*math.atan((x-DEN_JUMP)/DEN_GRAD)-AtX0);
      return dens1*N0
     end 
InitValue={
  n0= InitN0 
     ,
  B=function(x,y,z)
--[[  
      local omega_ci_x0 = 1/1.55*omega;
      local omega_ci_lx = 1/1.45*omega;
      local Bf_lx = omega_ci_lx*ionmass/ioncharge
      local Bf_x0 = omega_ci_x0*ionmass/ioncharge
--]]
      return {Btor,0,0}  
     end
      ,
  E=0.0, J=0.0
}
-- GFile
Grid=
{
  Type="CoRectMesh",
--  ScalarType="Complex",
  UnitSystem={Type="SI"},
  Topology=
  {       
      Type="3DCoRectMesh",
      Dimensions={NX,NY,NZ}, -- number of grid, now only first dimension is valid       
      GhostWidth= {2,0,0},  -- width of ghost points  , if gw=0, coordinate is 
                              -- Periodic at this direction          
  },
  Geometry=
  {
      Type="Origin_DxDyDz",
      Min={0.0,0.0,0.0},
      Max={LX,LY,LZ},
--      dt= 2.0*math.pi/omega_ci/100.0
      dt=0.5*LX/ (NX-1)/c  -- time step     
  },
  
}
--[[
Media=
{
   {Type="Vacuum",Region={{0.2*LX,0,0},{0.8*LX,0,0}},Op="Set"},

   {Type="Plasma",
     Select=function(x,y,z)
          return x>1.0 and x<2.0
        end
     ,Op="Set"},
}

Boundary={
   --   { Type="PEC", In="Vacuum",Out="NONE"},
   -- { Type="PEC", In="Plasma",Out="NONE"},
}
--]]

FieldSolver= 
{

   ColdFluid=
    {
     {Name="ion",m=1.0,Z=1.0,T= Ti,
       --n=function(x,y,z)   return InitN0(x,y,z)*0.5        end ,
       n=InitN0, J=0},
     {Name="ele",m=1.0/1836.2,Z=-1.0,T=Te,   n=InitN0, J=0}         
    }

}


CurrentSrc=
 { 
  Points={{0.2*LX,0.0,0.0},},
  Fun=function(x,y,z,t)
      local tau = t*omega_ci
      return {0,math.sin(tau)*(1-math.exp(-tau*tau)),0}   
      end
 }


-- The End ---------------------------------------

