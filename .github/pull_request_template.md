## Checklist
- [ ] CLA has been signed
- [ ] Code follows the project's style guidelines
- [ ] PR title tags are in the proper category
- [ ] Test scenario has been verified and passes

<!-- PR Title: [TagName] followed by a brief summary of the change -->
<!-- Example: [browsershell] Change to FreezingPolicy for suspend/resume request -->

## Detailed Notes
<!-- Carefully describe the details of this change below -->
<!-- Example:
This commit applies the FreezingPolicy to Neva PageContents.
When the tab becomes inactive, the tab will be suspended.
It was using blink::WebPagePauser to suspend the page, this commit changes it to use the FreezingPolicy.
When the tab becomes active again, the page will be resumed from the frozen state.
-->

## Test Scenario
<!-- Write the test steps starting from this line -->
<!-- If there are multiple test scenarios, write the most representative one -->
<!-- Example:
1. Launch any web app #1 and connect the app with DevTools.
2. On the Console tab of DevTools, type below commands:
   location = "https://page-lifecycle.glitch.me/"
   document.body.style.backgroundColor = "white";
3. Launch any web app #2 within 10 seconds.
4. Launch the web app #1 within 10 seconds.
5. Verify that the web app #1's state has changed to "frozen" and became "active".
-->

## Issue: N/A
<!-- Write "Issue:" followed by a space and then the Jira ticket number -->
<!-- If there are multiple issues, separate them with commas -->
<!-- If there is no related issue, enter "N/A" -->
<!-- Example: Issue: NEVA-xxxx, NEVA-xxxx -->
