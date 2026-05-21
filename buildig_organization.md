mill {

    class_effect : {
        target.set_numeric_variable(jobs/miller/count, +1);
    }

    instance_effect: {
        if cell.has_neighbour(river):
            target.set_numeric_variable(jobs/miller/count, +1)
    }
}

jobs

job_miller {
    class_effect {
        target.set_numeric_modirief("produces/core.wheat", +1)
    }
}

# Scenario 1: super mill, where one miller produces +3 wheat
bld_super_mill {
    class_effect {
        target.set_numeric(jobs/miller/count, 1)
        # Miller-director. Increases something
        target.set_numeric(jobs/miller.director/count, 1)
    }

    jobs_effect {
        miller: job_effect {
            target.set_numeric_modifier(core.wheat, +3)
        }
    }
}

How scopes look ?

```
scope building_number_5 {
    type: improvement_instance_scope
    parent: region_scope
    tags:
      # Inherits from its own class
      - scope {
          type: improvement_class_scope
          class: super_mill_class_scope
        }
      # some other tag
}
```

From that we can deduce what jobs we have
miller - 3
super-miller - 1

We want to creat scopes for these jobs
```
miller:
scope {
  parent: ?
  tags:
    # inherit from its own class
    - scope {
        class_miller
    }
    # We need to inherit from building-induced job-scope
    - scope {
        id: building_miller_jobs

        # That one in turns inherits from class_induced jobs?
        tags:
          - scope {
             building_miller_class_Jobs
          }

          # and from region-induced jobs?
          - scope {
              this_region_induced_job
          }
    }
}
```
Except now we have to have scope for every job_type x every region ?

How does job effect looks like?

effect {
  scope_type: SCOPE_TYPE_JOB
  selector:
    # We only target scopes that has tag origin=super_mill
    tag: origin=super_mill
  effect: target.set_numeric_variable(produces/wheat, +3)
}
