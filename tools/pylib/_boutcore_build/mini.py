from boutcore import *
init("-d mini -q -q -q")

class MyModel(PhysicsModel):
    def init(self,restart):
        self.n=create3D("dens:function")
        self.solve_for(dens=self.n)
    def rhs(self,time):
        self.n.ddt(DDX(self.n))

model=MyModel()
model.solve()

