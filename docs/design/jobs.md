# Jobs

## Jobs definitions

jobs schema is in proto/ruleset/jobs.proto file. Essentially jobs have
input and output. Input is what resources and in what quantity are consumed,
output is what resources and in what quantity are produced.


## Adding jobs

Adding jobs is done via effects, assigned to improvements. One can use
improvement class effects and improvement instance effects.

Variable for job with id 'JOB' is 'jobs/JOB/count'

### Example

**simple mill improvement**


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

**super mill**

One miller produces +3 wheat
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
