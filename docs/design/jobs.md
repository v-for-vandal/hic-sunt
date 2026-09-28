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


