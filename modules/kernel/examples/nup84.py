## \example kernel/nup84.py
## Modify the \ref modules/kernel/examples/nup84_cg.py "Nup84 CG"
## example by replacing a couple of the protein representations with
## higher resolution representations generated from PDB files. In
## addition, show how to visualize restraints and visualize the
## rejected conformations. Both are useful things to do when trying to
## figure out why optimization is not converging.

import IMP
import IMP.atom
import IMP.container
import IMP.display
import IMP.statistics
import IMP.example

# not finished
exit(0)

# First we define some basic parameters for the modeling effort

# the spring constant to use, it doesn't really matter
k=10
# the target resolution for the representation
resolution=100
# the box to perform everything in, make it flat as it is a 2D structure
bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-300,-300,-50),
                             IMP.algebra.Vector3D(300, 300, 50))

## Create a coarse grained protein with a given name, adding it to universe
def addd_protein(model, name, residues, parent, restraints, optimized_particles):
    ## Create a coarse grained protein with the passed residue information
    h = IMP.atom.create_protein(model, name, resolution, residues)

    parent.add_child(h)
    # Here, each of the constituent particles will be optimized independently
    optimized_particles.extend(h.get_children())

    ## Ensure that the various particles of the protein stay connected
    r=IMP.atom.create_connectivity_restraint([IMP.atom.Selection(c)\
                                              for c in h.get_children()], k,
        "connect "+name)

    if r:
        ## make sure there is an actual restraint
        restraints.append(r)
        r.set_maximum_score(k)

## Create protein as a rigid body from a pdb file
def add_protein_from_pdb(model, name, file, parent, restraints, optimized_particles):
    # we should keep the original particles around so they get written

    # create an atomic representation from the pdb file
    t=IMP.atom.read_pdb( IMP.get_example_path(os.path.join("data",file)), model,
                         IMP.atom.ATOMPDBSelector() )
    # extract the chain from the file
    c=IMP.atom.Chain(IMP.atom.get_by_type(t, IMP.atom.CHAIN_TYPE)[0])
    # there is no reason to use all atoms, just approximate the pdb shape instead
    s=IMP.atom.create_simplified_along_backbone(c, resolution/2.0, True)
    # tear down what is left
    IMP.atom.tear_down(t)
    # make the simplified structure rigid
    rb=IMP.atom.setup_as_rigid_body(s)
    rb.set_coordinates_are_optimized(True)
    optimized_particles.append(rb)
    parent.add_child(s)


## Create protein as a rigid body from several pdb file
def add_protein_from_pdbs(model, name, files, parent, restraints, optimized_particles):
    h= IMP.atom.Hierarchy.setup_particle(IMP.Particle(model, name))
    for i, f in enumerate(files):
        add_protein_from_pdb(model, name+str(i), f, h, restraints, optimized_particles)
    r=IMP.atom.create_connectivity_restraint([IMP.atom.Selection(c, hierarchy_types=[IMP.atom.FRAGMENT_TYPE]) for c in h.get_children()],
            k, "connect "+name)
    if r:
        restraints.append(r)
        r.set_maximum_score(k)

## Create all the needed representation using the above functions
def create_representation(model):
    restraints=[]
    optimized_particle=[]
    universe=IMP.atom.Hierarchy.setup_particle(IMP.Particle(m, "the universe"))

    add_protein(model, "Nup85", 570, universe, restraints, optimized_particles)

    # pin the c-terminus
    ct= IMP.atom.Selection(universe, molecule="Nup85",
                           hierarchy_types=[IMP.atom.FRAGMENT_TYPE],
                           terminus= IMP.atom.Selection.C)
    d= IMP.core.XYZ(ct.get_selected_particles()[0])
    d.set_coordinates(IMP.algebra.Vector3D(0,0,0))
    d.set_coordinates_are_optimized(False)

    add_protein("Nup84", 460, universe, restraints, optimized_particles)
    add_protein("Nup145C", 442, universe, restraints, optimized_particles)
    add_protein("Nup120", [0, 500, 761], universe, restraints, optimized_particles)
    add_protein("Nup133", [0, 450, 778, 1160], universe, restraints, optimized_particles)
    add_protein_from_pdb("Seh1", "seh1.pdb", universe, restraints, optimized_particles)
    add_protein_from_pdb("Sec13", "sec13.pdb", universe, restraints, optimized_particles)
    return universe, restraints, optimized_particles

def add_distance_restraint(selection0, selection1, name, restraints):
    r=IMP.atom.create_distance_restraint(selection0,selection1, 0, k, name)
    r.set_maximum_score(k)
    restraints.append(r)


def encode_data_as_restraints(universe, restraints):

    s0=IMP.atom.Selection(hierarchy=universe, hierarchy_types=[IMP.atom.FRAGMENT_TYPE],
                          molecule="Nup145C", residue_indexes=[(0,423)])
    s1=IMP.atom.Selection(hierarchy=universe, hierarchy_types=[IMP.atom.FRAGMENT_TYPE],
                          molecule="Nup84")
    s2=IMP.atom.Selection(hierarchy=universe, hierarchy_types=[IMP.atom.FRAGMENT_TYPE],
                          molecule="Sec13")
    r=create_connectivity_restraint([s0,s1,s2], k, "Nup145C Nup84 Sec13")
    r.set_maximum_score(k)
    restraints.append(r)

    add_distance_restraint(IMP.atom.Selection(hierarchy=universe, molecule="Nup145C",
                                              residue_indexes=[(0,423)],
                                              hierarchy_types=[IMP.atom.FRAGMENT_TYPE]),
                           IMP.atom.Selection(hierarchy=universe, molecule="Nup85",
                                              hierarchy_types=[IMP.atom.FRAGMENT_TYPE]),
                           "Num145C, Nup85", restraints)
    add_distance_restraint(IMP.atom.Selection(hierarchy=universe, molecule="Nup145C",
                                              residue_indexes=[(0,423)],
                                              hierarchy_types=[IMP.atom.FRAGMENT_TYPE]),
                           IMP.atom.Selection(hierarchy=universe, molecule="Nup120",
                                              residue_indexes= [(500, 762)],
                               hierarchy_types=[IMP.atom.FRAGMENT_TYPE]),
                                    "Nup145C Nup120", restraints)
    add_distance_restraint(IMP.atom.Selection(hierarchy=universe, molecule="Nup84",
                                              hierarchy_types=[IMP.atom.FRAGMENT_TYPE]),
                           IMP.atom.Selection(hierarchy=universe, molecule="Nup133",
                                              residue_indexes=[(778, 1160)],
                               hierarchy_types=[IMP.atom.FRAGMENT_TYPE]),
                            "Nup84 Nup133", restraints)
    add_distance_restraint(IMP.atom.Selection(hierarchy=universe, molecule="Nup85",
                                              hierarchy_types=[IMP.atom.FRAGMENT_TYPE]),
                           IMP.atom.Selection(hierarchy=universe, molecule="Seh1",
                                              hierarchy_types=[IMP.atom.FRAGMENT_TYPE]),
                           "Nup85 Seh1", restraints)
    add_distance_restraint(IMP.atom.Selection(hierarchy=universe, molecule="Nup145C",
                                              residue_indexes=[(0,423)],
                                              hierarchy_types=[IMP.atom.FRAGMENT_TYPE]),
                           IMP.atom.Selection(hierarchy=universe, molecule="Sec13",
                                              hierarchy_types=[IMP.atom.FRAGMENT_TYPE]),
                           "Nup145C Sec13", restraints)



# find acceptable conformations of the model
def get_configurations(model, restraints, optimized_particles):
    representation= IMP.atom.Selection(optimized_particles, hierarchy_types=[IMP.atom.FRAGMENT_TYPE])
    #cpc= IMP.container.ClosePairContainer(representation.get_particles(), 0, 10)
    #evr= IMP.container.PairRestraint(IMP.core.SoftSpherePairScore(k), cpc,
    #                                 "Excluded Volume")
    scale=.5
    mc= IMP.core.MonteCarlo(m)
    movers=[]
    for p in optimized_particles:
        if IMP.core.RigidBody.particle_is_instance(p):
            mover= IMP.core.RigidBodyMover(p, IMP.core.XYZR(p).get_radius()*scale,
                                           .2*scale)
            movers.append(mover)
        else:
            mover= IMP.core.BallMover(p, IMP.core.XYZR(p).get_radius()*scale)
            movers.append(mover)
    serial_mover= IMP.core.SerialMover(movers)
    mc.add_mover(serial_mover)
    scoring_function= IMP.core.IncrementalScoringFunction(restriants)
    scoring_function.add_close_pair_score(IMP.core.SoftSpherePairScore(k), representation)

    configuration_set= IMP.ConfigurationSet(model)
    HERE
    # must write our own sampler as IMP.core.MCCGSampler doesn't handle rigid bodies
    return configuration_set


model= IMP.Model()
universe, restraints, optimized_particles= create_representation(model)
encode_data_as_restraints(universe, restraints)

configuration_set= get_configurations(model, restraints,
                                      optimized_particles)

print "Found", configuration_set.get_number_of_configurations(), "good configurations"

# now lets save them all to a file
rmf_file_name=IMP.base.create_temporary_file_name("nup84", ".rmf")
rmf= RMF.create_rmf_file(rmf_file_name)

# we want to see the scores of the various restraints also
IMP.rmf.add_restraints(rmf, restraints)
# and the actual structures
IMP.rmf.add_hierarchy(rmf, universe)

for i in range(0, configuration_set.get_number_of_configurations()):
    configuration_set.load_configuration(i)
    # align all the configurations with the first so they display nicely
    # if we want to be fancy we can account for flips too
    if i==0:
        base_coords=[IMP.core.XYZ(p).get_coordinates() for p in optimized_particles]
    else:
        tr= IMP.algebra.get_transform_taking_first_to_second(optimized_particles, base_coords)
        IMP.core.transform(optimized_particles, tr)
    # update the restraint scores
    sf.evaluate(False)
    IMP.rmf.save_frame(rmf, i)

print "You can now open", rmf_file_name, "in chimera"
