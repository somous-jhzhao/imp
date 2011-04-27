import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.membrane
import math

from membrane_parameters import *

def create_restraints(m, chain, tbr, TMH, rot0, topo):

    def add_excluded_volume():
        lsc= IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s=IMP.atom.Selection(chain, residue_indexes=[(h[0],h[1]+1)])
            lsc.add_particles(s.get_selected_particles())
        evr=IMP.core.ExcludedVolumeRestraint(lsc,kappa_)
        m.add_restraint(evr)
        m.set_maximum_score(evr, max_score_)

    def add_loop_restraint(s0, s1, x0, k):
        hub= IMP.core.HarmonicUpperBound(x0,k)
        df= IMP.core.DistancePairScore(hub)
        dr= IMP.core.PairRestraint(df, IMP.ParticlePair(s0, s1))
        m.add_restraint(dr)
        m.set_maximum_score(dr, max_score_)
        return dr

    def add_packing_restraint():
## if the helices are interacting, apply a filter on the crossing angle
## first define the allowed intervals, by specifying the center
## of the distributions (Walters and DeGrado PNAS (2007) 103:13658)
        om0=(-156.5, 146.4, -37.9, 13.8, 178.0, 25.5, -161.1, 44.8, 127.4, -60.2, -129.2, 2.4, 161.0)
#  the sigmas
        sig_om0=(10.1, 13.6, 7.50, 16.6, 20.8, 11.2, 10.3, 8.8, 12.3, 14.8, 12.9, 16.2, 17.6)
#  distance cutoff
        dd0=(8.61, 8.57, 7.93, 9.77, 9.14, 8.55, 9.30, 7.96, 9.40, 8.61, 8.97, 8.55, 8.75)
#  and distance sigmas
        sig_dd0=(0.89, 0.99, 0.88, 1.18, 1.47, 1.05, 1.57, 1.13, 1.0, 1.04, 1.65, 0.78, 1.33)
#  the allowed number of sigma
        nsig=packing_nsig_
#  and the number of clusters
        ncl=packing_ncl_
# create allowed intervals (omega in radians)
# and control periodicity
        om_b=[]; om_e=[]; dd_b=[]; dd_e=[]
        for i in range(ncl):
            omb=math.radians(om0[i]-nsig*sig_om0[i])
            ome=math.radians(om0[i]+nsig*sig_om0[i])
            ddb=dd0[i]-nsig*sig_dd0[i]
            dde=dd0[i]+nsig*sig_dd0[i]
            om_b.append(max(omb,-math.pi))
            om_e.append(min(ome,math.pi))
            dd_b.append(ddb)
            dd_e.append(dde)
            if ( omb < -math.pi ):
                om_b.append(2*math.pi+omb)
                om_e.append(math.pi)
                dd_b.append(ddb)
                dd_e.append(dde)
            if ( ome > math.pi ):
                om_b.append(-math.pi)
                om_e.append(-2*math.pi+ome)
                dd_b.append(ddb)
                dd_e.append(dde)
        lrb= IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
            rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
            lrb.add_particle(rb)
        nrb= IMP.container.ClosePairContainer(lrb, 25.0)
        ps=  IMP.membrane.RigidBodyPackingScore(tbr, om_b, om_e, dd_b, dd_e)
        prs= IMP.container.PairsRestraint(ps, nrb)
        m.add_restraint(prs)
        m.set_maximum_score(prs, max_score_)

    def add_DOPE():
        IMP.atom.add_dope_score_data(chain)
        dsc=IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_indexes=[(h[0],h[1]+1)])
            dsc.add_particles(s.get_selected_particles())
        dpc=IMP.container.ClosePairContainer(dsc, 15.0, 0.0)
        f=IMP.membrane.SameHelixPairFilter()
        dpc.add_pair_filter(f)
        dps=IMP.atom.DopePairScore(15.0, IMP.atom.get_data_path(score_name_))
        dope=IMP.container.PairsRestraint(dps, dpc)
        m.add_restraint(dope)
        m.set_maximum_score(dope, max_score_)

    def add_interacting_restraint(h0, h1):
        s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h0[0])
        s1=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h1[0])
        rb0=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        rb1=IMP.core.RigidMember(s1.get_selected_particles()[0]).get_rigid_body()
        hub= IMP.core.HarmonicUpperBound(8.0, kappa_)
        sd=  IMP.core.DistancePairScore(hub)
        kc=  IMP.core.KClosePairsPairScore(sd,tbr,1)
        ir=  IMP.core.PairRestraint(kc,[rb0,rb1])
        m.add_restraint(ir)
        m.set_maximum_score(ir, max_score_)
        return ir

    def add_diameter_restraint(diameter):
        lrb= IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
            rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
            lrb.add_particle(rb)
        hub=IMP.core.HarmonicUpperBound(0, kappa_)
        dr=IMP.core.DiameterRestraint(hub, lrb, diameter)
        m.add_restraint(dr)
        m.set_maximum_score(dr, max_score_)

    def add_depth_restraint(range):
        lrb= IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
            rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
            lrb.add_particle(rb)
        well=IMP.core.HarmonicWell(range, kappa_)
        ass=IMP.core.AttributeSingletonScore(well,IMP.FloatKey("z"))
        sr=IMP.container.SingletonsRestraint(ass, lrb)
        m.add_restraint(sr)
        m.set_maximum_score(sr, max_score_)

    def add_tilt_restraint(range0,rot0):
        laxis=(1.0,0.0,0.0)
        zaxis=(0.0,0.0,1.0)
        for i,h in enumerate(TMH):
            s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
            rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
            if ( rot0[i] > 0 ): range=[math.pi-range0[1],math.pi-range0[0]]
            else              : range=[range0[0],range0[1]]
            well=IMP.core.HarmonicWell(range, kappa_)
            tss=IMP.membrane.TiltSingletonScore(well, laxis, zaxis)
            sr=IMP.core.SingletonRestraint(tss, rb)
            m.add_restraint(sr)
            m.set_maximum_score(sr, max_score_)

# assembling all the restraints
    rset=IMP.RestraintSet()
    add_excluded_volume()
    for i in range(len(TMH)-1):
        s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = TMH[i][1])
        s1=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = TMH[i+1][0])
        p0=s0.get_selected_particles()[0]
        p1=s1.get_selected_particles()[0]
        rb0=IMP.core.RigidMember(p0).get_rigid_body()
        rb1=IMP.core.RigidMember(p1).get_rigid_body()
        length=1.6*(TMH[i+1][0]-TMH[i][1]+1)+7.4
        lr=add_loop_restraint(p0,p1,length,1000)
        rset.add_restraint(lr)
    add_packing_restraint()
    add_DOPE()
    add_diameter_restraint(diameter_)
    add_depth_restraint(z_range_)
    add_tilt_restraint(tilt_range_,rot0)
    ir=add_interacting_restraint(TMH[1],TMH[2])
    rset.add_restraint(ir)
    return rset
