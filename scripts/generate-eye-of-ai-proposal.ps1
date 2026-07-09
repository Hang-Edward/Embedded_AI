param(
    [string]$TemplatePath = "D:\Desktop\MedicalMenagementSystem proposal.docx",
    [string]$OutputPath = "D:\VScode Projects\Embedded_AI\outputs\The_Eye_of_AI_Proposal.docx"
)

$ErrorActionPreference = "Stop"

function Set-ParagraphStyle {
    param(
        [Parameter(Mandatory = $true)] $Selection,
        [string]$FontName,
        [double]$FontSize,
        [bool]$Bold = $false,
        [bool]$Italic = $false,
        [int]$Alignment = 3,
        [double]$SpaceBefore = 0,
        [double]$SpaceAfter = 0,
        [double]$FirstLineIndent = 0,
        [double]$LeftIndent = 0,
        [int]$LineSpacingRule = 1,
        [double]$LineSpacing = 18
    )

    $Selection.Font.Name = $FontName
    $Selection.Font.Size = $FontSize
    $Selection.Font.Bold = if ($Bold) { 1 } else { 0 }
    $Selection.Font.Italic = if ($Italic) { 1 } else { 0 }
    $Selection.Font.Color = 0

    $Selection.ParagraphFormat.Alignment = $Alignment
    $Selection.ParagraphFormat.SpaceBefore = $SpaceBefore
    $Selection.ParagraphFormat.SpaceAfter = $SpaceAfter
    $Selection.ParagraphFormat.FirstLineIndent = $FirstLineIndent
    $Selection.ParagraphFormat.LeftIndent = $LeftIndent
    $Selection.ParagraphFormat.LineSpacingRule = $LineSpacingRule
    $Selection.ParagraphFormat.LineSpacing = $LineSpacing
}

function Add-Paragraph {
    param(
        [Parameter(Mandatory = $true)] $Selection,
        [string]$Text,
        [string]$FontName,
        [double]$FontSize,
        [bool]$Bold = $false,
        [bool]$Italic = $false,
        [int]$Alignment = 3,
        [double]$SpaceBefore = 0,
        [double]$SpaceAfter = 0,
        [double]$FirstLineIndent = 0,
        [double]$LeftIndent = 0,
        [int]$LineSpacingRule = 1,
        [double]$LineSpacing = 18
    )

    Set-ParagraphStyle -Selection $Selection -FontName $FontName -FontSize $FontSize -Bold $Bold -Italic $Italic `
        -Alignment $Alignment -SpaceBefore $SpaceBefore -SpaceAfter $SpaceAfter -FirstLineIndent $FirstLineIndent `
        -LeftIndent $LeftIndent -LineSpacingRule $LineSpacingRule -LineSpacing $LineSpacing

    if ([string]::IsNullOrEmpty($Text)) {
        $Selection.TypeParagraph()
    } else {
        $Selection.TypeText($Text)
        $Selection.TypeParagraph()
    }
}

function Add-Image {
    param(
        [Parameter(Mandatory = $true)] $Selection,
        [Parameter(Mandatory = $true)] [string]$ImagePath,
        [double]$WidthPoints = 360,
        [int]$Alignment = 1,
        [double]$SpaceAfter = 6
    )

    if (-not (Test-Path -LiteralPath $ImagePath)) {
        return
    }

    Set-ParagraphStyle -Selection $Selection -FontName 'Times New Roman' -FontSize 12 -Alignment $Alignment -SpaceAfter $SpaceAfter
    $shape = $Selection.InlineShapes.AddPicture($ImagePath)
    $shape.LockAspectRatio = -1
    $shape.Width = $WidthPoints
    $Selection.TypeParagraph()
}

function Add-SeparatorBreak {
    param([Parameter(Mandatory = $true)] $Selection)
    Add-Paragraph -Selection $Selection -Text "" -FontName 'Times New Roman' -FontSize 12 -SpaceAfter 0
}

$outputDir = Split-Path -Parent $OutputPath
if (-not (Test-Path -LiteralPath $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir | Out-Null
}

$figureRoot = "D:\VScode Projects\Embedded_AI\outputs\ppt_review_doc_images"
$figureEvolution = Join-Path $figureRoot "image35.png"
$figureBridge = Join-Path $figureRoot "image36.png"
$figureArchitecture = Join-Path $figureRoot "image37.png"
$figureWorkflow = Join-Path $figureRoot "image38.png"
$figureBoundary = Join-Path $figureRoot "image39.png"
$figureEvidence = Join-Path $figureRoot "image40.png"
$figureValidation = Join-Path $figureRoot "image41.png"

$word = $null
$doc = $null

try {
    $word = New-Object -ComObject Word.Application
    $word.Visible = $false

    $doc = $word.Documents.Open($TemplatePath)
    $doc.SaveAs([ref]$OutputPath)

    $doc.Content.Delete()
    $selection = $word.Selection

    $selection.TypeParagraph()
    $selection.TypeParagraph()
    Add-Paragraph -Selection $selection -Text "The Eye of AI: Embedded AI Reality Bridge" -FontName 'Times New Roman' -FontSize 22 -Bold $true -Alignment 3 -LineSpacingRule 5 -LineSpacing 13.2 -SpaceAfter 7.8
    Add-Paragraph -Selection $selection -Text "AI之眼：连接现实世界的多模态 AI 原型机" -FontName '等线' -FontSize 16 -Bold $true -Alignment 1 -LineSpacingRule 5 -LineSpacing 13.2 -SpaceAfter 7.8

    1..5 | ForEach-Object {
        $selection.TypeParagraph()
    }

    Add-Paragraph -Selection $selection -Text 'Group member(s): ________________________________' -FontName 'Times New Roman' -FontSize 12 -Bold $true -Alignment 1 -LineSpacingRule 5 -LineSpacing 13.2 -SpaceAfter 7.8

    1..4 | ForEach-Object {
        $selection.TypeParagraph()
    }

    Add-Paragraph -Selection $selection -Text "1.1 The Background and Motivation of System" -FontName 'Times New Roman' -FontSize 14 -Bold $true -Alignment 3 -SpaceAfter 7.8
    Add-Paragraph -Selection $selection -Text "After a long cycle of development, AI has evolved from IDE-based code completion tools, to web chatbots such as ChatGPT, and then to desktop agents represented by Codex that can read files, browse the web, and manipulate local workspaces. However, most AI systems are still trapped inside the computer screen. They can reason about the world, but they cannot directly perceive and respond to the physical world around users. Based on this limitation, our project proposes The Eye of AI, a multimodal embedded prototype that gives AI an 'eye' and a 'bridge' to reality. Instead of building a final smart-glasses product, we focus on validating the core abilities that smart glasses should possess: scene perception, voice interaction, physical triggering, cloud reasoning, and local feedback. By combining Raspberry Pi, NUCLEO, camera, microphone, LCD, status light, keypad, Qwen, and DeepSeek, the system enables AI to observe the current environment, understand speech commands, make decisions, and return synchronized feedback on embedded hardware and desktop software. Therefore, the project demonstrates a meaningful transition from screen-bound AI to embodied AI interaction." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -FirstLineIndent 21 -LineSpacing 18
    Add-Image -Selection $selection -ImagePath $figureEvolution -WidthPoints 360 -Alignment 1
    Add-Image -Selection $selection -ImagePath $figureBridge -WidthPoints 360 -Alignment 1

    Add-Paragraph -Selection $selection -Text "1.2 System Objectives" -FontName 'Times New Roman' -FontSize 14 -Bold $true -Alignment 3 -SpaceAfter 7.8
    Add-Paragraph -Selection $selection -Text "1.2.1 Basic Process" -FontName 'Times New Roman' -FontSize 12 -Bold $true -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "The subject of the system is an embedded multimodal AI prototype. The basic process of using the system is: physical trigger - voice capture - scene capture - multimodal understanding - final reasoning - embedded/Desktop feedback - ready for the next trigger." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -FirstLineIndent 21 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Trigger:" -FontName 'Times New Roman' -FontSize 12 -Bold $true -Alignment 3 -LeftIndent 27.25 -FirstLineIndent -20.15 -SpaceBefore 4.65 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "The user presses the three-key keypad K-B or the NUCLEO blue button to start a new interaction cycle. The trigger is transmitted to the Raspberry Pi bridge through serial communication." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Perception:" -FontName 'Times New Roman' -FontSize 12 -Bold $true -Alignment 3 -LeftIndent 27.25 -FirstLineIndent -20.15 -SpaceBefore 4.65 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "After the trigger, the Raspberry Pi activates Logitech C270 to capture voice and image data. Qwen ASR converts speech to text, and Qwen Vision interprets the current frame when visual context is required." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Reasoning:" -FontName 'Times New Roman' -FontSize 12 -Bold $true -Alignment 3 -LeftIndent 27.25 -FirstLineIndent -20.15 -SpaceBefore 4.65 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "DeepSeek receives the text-only or multimodal prompt and is responsible for final answer composition, continuous dialogue context, and high-level decision making." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Feedback:" -FontName 'Times New Roman' -FontSize 12 -Bold $true -Alignment 3 -LeftIndent 27.25 -FirstLineIndent -20.15 -SpaceBefore 4.65 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "The system outputs synchronized results to the Raspberry Pi terminal, the LCD attached to the prototype, the status light, and the Windows Control Center. The user can then trigger another round, forming a complete interaction loop." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "The desktop control center functions are as follows." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LeftIndent 18 -FirstLineIndent -18 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Real-time conversation area: display synchronized user input, current image, and final AI answer." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LeftIndent 18 -FirstLineIndent -18 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "History area: save completed sessions and allow users to resume an earlier conversation." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LeftIndent 18 -FirstLineIndent -18 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Connection diagnostics area: inspect SSH, serial, camera, microphone, API configuration, LCD, keypad, and recent files." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LeftIndent 18 -FirstLineIndent -18 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Agent interaction area: accept free-form user requests and support both plain-text and multimodal question answering." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LeftIndent 18 -FirstLineIndent -18 -LineSpacing 18

    Add-Paragraph -Selection $selection -Text "1.2.2 Flowchart of partial function implementation" -FontName 'Times New Roman' -FontSize 12 -Bold $true -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text '(A) Deployment topology of the prototype system.' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Image -Selection $selection -ImagePath $figureArchitecture -WidthPoints 360 -Alignment 1
    Add-Paragraph -Selection $selection -Text '(B) Single interaction workflow and fallback path.' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Image -Selection $selection -ImagePath $figureWorkflow -WidthPoints 360 -Alignment 1
    Add-Paragraph -Selection $selection -Text '(C) Prototype scope and current boundary of validated functions.' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Image -Selection $selection -ImagePath $figureBoundary -WidthPoints 360 -Alignment 1
    Add-Paragraph -Selection $selection -Text '(D) Demonstration evidence of hardware setup, captured scene, and system feedback.' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Image -Selection $selection -ImagePath $figureEvidence -WidthPoints 360 -Alignment 1

    Add-Paragraph -Selection $selection -Text "1.2.3 Other objectives." -FontName 'Times New Roman' -FontSize 12 -Bold $true -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text '(A) Interface design objectives.' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Page content: The desktop application should present the AI workflow clearly, keep the conversation area immersive, and keep hardware diagnosis readable in presentation scenarios." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LeftIndent 18 -FirstLineIndent -18 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Navigation structure: The page must allow users to switch quickly among real-time dialogue, history, camera view, logs, settings, and diagnostics." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LeftIndent 18 -FirstLineIndent -18 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Technical environment: The software should run on Windows and communicate with Raspberry Pi through SSH while preserving readability and stable rendering." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LeftIndent 18 -FirstLineIndent -18 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Artistic style: The interface should be modern and visually impressive, because one of the project goals is to make the AI-to-reality bridge intuitive during classroom demonstrations." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LeftIndent 18 -FirstLineIndent -18 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text '(B) Response requirements:' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "When a user triggers the system, the prototype should enter the recording state immediately, provide visible status feedback, and return the final answer within an acceptable interactive delay under normal network conditions. The desktop side should keep the display synchronized with the Raspberry Pi side." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -FirstLineIndent 21 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text '(C) Security and persistence requirements:' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "The system should separate API key files from the source repository, keep chat sessions and captured media readable for demonstration review, and maintain stable recovery when SSH, serial, or model calls fail." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -FirstLineIndent 21 -LineSpacing 18
    Add-Image -Selection $selection -ImagePath $figureValidation -WidthPoints 360 -Alignment 1

    Add-Paragraph -Selection $selection -Text "1.2.4 Diagram of C++ inheritance used in the project." -FontName 'Times New Roman' -FontSize 12 -Bold $true -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "The project intentionally uses object-oriented C++ design to satisfy coursework requirements while preserving real engineering value. Abstract interfaces such as AiVisionService, AudioRecorder, and OutputDevice define extensible device or service contracts, while concrete subclasses such as QwenVisionService and ShellAudioRecorder provide platform-specific implementations. This design also supports polymorphic expansion for future hardware modules." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -FirstLineIndent 21 -LineSpacing 18
    Add-Image -Selection $selection -ImagePath $figureArchitecture -WidthPoints 320 -Alignment 1

    Add-Paragraph -Selection $selection -Text "2.1 Key Issues for System" -FontName 'Times New Roman' -FontSize 14 -Bold $true -Alignment 3 -SpaceAfter 7.8
    Add-Paragraph -Selection $selection -Text '(A) The coordination of multimodal perception and cloud reasoning' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "The most essential problem is how to let hardware-triggered real-world inputs be converted into stable multimodal prompts. The system must coordinate camera capture, microphone recording, Qwen ASR, Qwen Vision, and DeepSeek so that the final answer remains coherent and useful." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -FirstLineIndent 21 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text '(B) The cooperation among Raspberry Pi, NUCLEO, and desktop software' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "The second key issue is distributed coordination. Raspberry Pi handles runtime logic and model calls, NUCLEO handles physical triggering and local feedback, and the Windows Control Center acts as the supervision surface. The challenge is to ensure that the three layers remain synchronized and debuggable." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -FirstLineIndent 21 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text '(C) The design of desktop agent interaction' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "Since the project demonstrates not only hardware but also an AI desktop experience, the interface must support conversation history, rendering, diagnostics, and multimodal reasoning without becoming a simple static showcase page. It should feel like a real agent workbench." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -FirstLineIndent 21 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text '(D) Future Prospect' -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -LineSpacing 18
    Add-Paragraph -Selection $selection -Text "The current work is a reproducible prototype instead of a final smart-glasses device. In future work, the same interaction loop can be migrated into a smaller wearable form, with onboard networking, better power management, lighter sensors, and more compact human-computer interaction modules." -FontName 'Times New Roman' -FontSize 12 -Alignment 3 -FirstLineIndent 21 -LineSpacing 18

    Add-Paragraph -Selection $selection -Text "2.2 Duty Assignments" -FontName 'Times New Roman' -FontSize 14 -Bold $true -Alignment 0 -SpaceAfter 7.8

    $table = $doc.Tables.Add($doc.Range($doc.Content.End - 1, $doc.Content.End - 1), 5, 2)
    $table.Style = "网格型"
    $table.Borders.Enable = 1
    $table.Cell(1, 1).Range.Text = "Person"
    $table.Cell(1, 2).Range.Text = "Duty"
    $table.Cell(2, 1).Range.Text = "Member A"
    $table.Cell(2, 2).Range.Text = "System design, project integration, and proposal/report coordination."
    $table.Cell(3, 1).Range.Text = "Member B"
    $table.Cell(3, 2).Range.Text = "Raspberry Pi bridge, Qwen / DeepSeek API workflow, camera and microphone integration."
    $table.Cell(4, 1).Range.Text = "Member C"
    $table.Cell(4, 2).Range.Text = "NUCLEO firmware, keypad / LCD / status light control, and serial protocol design."
    $table.Cell(5, 1).Range.Text = "Member D"
    $table.Cell(5, 2).Range.Text = "Windows Control Center, session history, diagnostics, UI polishing, and testing."

    foreach ($row in 1..5) {
        foreach ($col in 1..2) {
            $cellRange = $table.Cell($row, $col).Range
            $cellRange.Font.Name = 'Times New Roman'
            $cellRange.Font.Size = 12
            $cellRange.Font.Bold = 0
            $cellRange.ParagraphFormat.Alignment = 3
            $cellRange.ParagraphFormat.LineSpacingRule = 1
            $cellRange.ParagraphFormat.LineSpacing = 18
        }
    }

    $table.Cell(1, 1).Range.Font.Bold = 1
    $table.Cell(1, 2).Range.Font.Bold = 1

    $selection.MoveDown() | Out-Null
    $selection.TypeParagraph()

    $doc.Save()

    try {
        $pdfPath = [System.IO.Path]::ChangeExtension($OutputPath, ".pdf")
        $doc.ExportAsFixedFormat($pdfPath, 17)
    } catch {
        # PDF 导出失败时保留 docx 即可
    }
}
finally {
    if ($doc -ne $null) {
        $doc.Close()
        [System.Runtime.InteropServices.Marshal]::ReleaseComObject($doc) | Out-Null
    }
    if ($word -ne $null) {
        $word.Quit()
        [System.Runtime.InteropServices.Marshal]::ReleaseComObject($word) | Out-Null
    }
    [GC]::Collect()
    [GC]::WaitForPendingFinalizers()
}

