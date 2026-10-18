# Explicit native Develop guide acceptance

This Development automation entry invokes the complete published guides in an
already installed/configured official project. It does not create a player.
After the main agent's Develop Go, create the owned guest through the ordinary
SDK/UI, retain its original identity and use the same isolated profile for restore.

Run the fixed test `Seagull.Acceptance.DevelopPublicGuide` with private environment
paths `SEAGULL_DEVELOP_MANIFEST` and `SEAGULL_DEVELOP_COMMAND`. The private SDK
configuration must contain exactly `http://127.0.0.1:3170` and match the manifest's
project key. No credential is passed as a command-line value or written to source.
The manifest uses the shared native fields: stage=develop, rootApproved=true,
baseUrl, projectId, appApiKey, controlDirectory and optional allowWrites.

The original command is `<controlDirectory>/<id>.json` and contains canonical UUID
`id`, action=guide, feature and the expected existing `userId`. Its permanent
`.started` marker prevents automatic dispatch after a process restart. The native
launcher must save/fsync the complete original command and parent directory before
starting the process. Preserve an uncertain original; do not remove its marker.

All eleven guide names are accepted: player-values, statistics, achievements, inbox,
friends, groups, items, shared-objects, matches, chat and marketplace. Statistics
and Achievements also require manifest allowWrites=true and command confirm=true;
Chat requires channelId and Marketplace listingId. Only the existing complete
public methods run. Their own journals retain write originals before dispatch.

Results are saved under the private control directory's results folder, with first
actual feature HTTP status/time, final status and response count. This is a prepared
SDK/project/restored-session measurement. It does not measure fresh installation
and does not replace DTO/business-state reconciliation or the official Game/Hello
E4/E5 paths. Missing approval/configuration/session fails closed before feature HTTP.
