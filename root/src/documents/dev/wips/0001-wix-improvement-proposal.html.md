---
wip: 0001
type: Process
author: Rob Mensching (rob at firegiant.com)
title: WiX Improvement Proposal
---

    [Bob] Should this have a feature request associated with it (instead of being 0001)?

## User stories

* As a WiX developer I can document changes I plan to make to the WiX toolset features or processes such that other developers can discuss the proposal.

    [Bob] So pull requests will let anyone participate (which is good). But iterating on them is a pain,
    at least on Codeplex. I'm guessing we'll end up mostly talking on 

* As a WiX developer I can summarize all of the discussion about an improvement such that future developers and users can find the results in one document.

## Proposal

A WiX Improvement Proposal, or WIP, is a document that defines a process or feature
improvement in the WiX toolset. Not every change to the WiX toolset must have a WIP. Bug
fixes, for example, are often sufficiently defined in the [issue tracker]. For
everything else the WIP process is intended to be lightweight process that provides value
for developers today and tomorrow.

    [Bob] I'm thinking we need a checklist/task list here instead of the following paragraph. 
    For example, a thread on wix-devs should be high on the list. Also, we need to 
    discuss the workflow for approval. Something like 
    http://www.python.org/dev/peps/pep-0001/#submitting-a-pep and
    http://www.python.org/dev/peps/pep-0001/#pep-review-resolution except a lot more terse
    (though who doesn't love a state diagram?!). What do you think?

    Usually, it is a good idea to discuss a new feature on the wix-devs mailing list to gauge interest.
    If there is interest in the idea, then it is captured in a feature request on the WiX toolset 
    [issue tracker]. The standard triage processes will schedule the feature in an appropriate release.

## Creating a WiX Improvement Proposal

The feature request number from the [issue tracker] also serves as the unique WIP identifier. Using
the same identifier lets us use the issue tracker to schedule (and possibly re-schedule) the feature request with
all the other bugs going into a release. It is up to you to ensure that
you've entered the correct number as the WIP identifier.

With WIP identifier in hand you can now get started on the WIP. First, enlist in the `wixweb` branch
of the WiX toolset's Git repository. A template WIP file exists in the `root\src\documents\dev\wips\`
folder called `0000-template-wix-improvement-proposal.html.md`. Copy that file, replacing the four
zeros with the WIP identifier. The remainder of the file name should be the title of the WIP
all lowercase with dashes separating the words. Finally, the file must end with `.html.md` to be
processed correctly.

### WIP Metadata

Open your new WIP document and update the metadata at the top of the file. The `wip:` value 
should contain the WIP identifier.

The `type:` should be `Feature` when adding or updating
functionality in the WiX toolset or `Process` when updating the development process for the
WiX toolset.

The `author:` metadata should contain your name and any contributors names (comma delimited). 
Your email address address should also be provided to uniquely identify you from others that
may have the same name (however unlikely that may be).

Finally, update the `title:` of the WIP to match the name of the file using title casing.

### WIP User Stories

The WIP starts with user stories. Follow the standard user story format "As a [role], I can
[accomplish task thanks to proposal] such that [benefit from proposal]." There are a couple
standard roles in the WiX toolset:

* *WiX developer* - someone like you that works on the WiX toolset itself to improve it.

* *Setup developer* - a user of the WiX toolset that builds installation packages.

Ideally, there is more than one user story.

### WIP Proposal

The proposal section of the WIP is a fairly freeform description of the feature or process
improvement. Provide the motivation for the proposal. Describe how the proposal works. List examples,
particularly source code, that demonstrates how the improvement will be used. Basically, provide
the bulk of the detail about the WIP here.

### WIP Considerations

As issues with the proposal come to mind or are raised by reviewers, capture them in the
considerations section of the WIP. As appropriate, explain the trade offs of the proposed
design and why a particular decision was made. This section may provide the most value for
developers and users that follow later so do not be afraid to list everything that comes
to mind. Rare is it that a proposal doesn't have some negatives to consider.

## WIP Workflow

Like the code of the WiX toolset, WIPs are designed to be living documents. So do not be afraid
to "check in early and often". Commit your changes to the WIP and send a pull request any time
you feel the update provides enough value. The WIP update pull requests will be accepted like
any code submission, with a very low review bar (essentially, as long as the update is coherent
we'll integrate it for discussion).

WIP documents can be discussed anywhere (mailing list, online meetings, twitter, etc) but everyone
is encouraged to contribute their ideas to the proposal by editing the document or working with
the author to update the document.

## Considerations

The WIP process is designed to add value with as little overhead as possible. Of course, it takes
effort to write down how a feature should work. But often the process of writing down how to solve
a problem, issues are uncovered that were not initially considered.

More than anything capturing the process adds tremendous value to those that value. So, while it
takes a bit more time up front, we think the WIPs will add a lot of value over the long term.

Finally, this is not a college writing assignment. Use as many words as necessary to capture the
idea. No need to write a book. Also, feel free to enlist others to help you write the document.
Start with a rough draft and have someone help flesh out the missing details.

  [issue tracker]: /issues/
