object RegRipper: TRegRipper
  Left = 317
  Top = 158
  Width = 637
  Height = 533
  Caption = 'Registrar Ripper'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = OnClose
  OnCreate = OnCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Source: TGroupBox
    Left = 8
    Top = 8
    Width = 617
    Height = 81
    Caption = 'Source'
    TabOrder = 2
    object Label1: TLabel
      Left = 8
      Top = 16
      Width = 69
      Height = 13
      Caption = 'Subjects URL:'
    end
    object Label2: TLabel
      Left = 8
      Top = 48
      Width = 88
      Height = 13
      Caption = 'Departments URL:'
    end
  end
  object SubjURL: TEdit
    Left = 112
    Top = 24
    Width = 505
    Height = 21
    TabOrder = 0
    Text = 'http://www.columbia.edu/cu/bulletin/uwb/subj/'
  end
  object DeptURL: TEdit
    Left = 112
    Top = 56
    Width = 505
    Height = 21
    TabOrder = 1
    Text = 'http://www.columbia.edu/cu/bulletin/uwb/sel/'
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 96
    Width = 617
    Height = 73
    Caption = 'Destination'
    TabOrder = 3
    object Label3: TLabel
      Left = 8
      Top = 24
      Width = 43
      Height = 13
      Caption = 'Text File:'
    end
    object filename: TEdit
      Left = 104
      Top = 24
      Width = 417
      Height = 21
      TabOrder = 0
    end
    object browse: TButton
      Left = 528
      Top = 24
      Width = 75
      Height = 25
      Caption = 'Browse...'
      TabOrder = 1
      OnClick = browseClick
    end
  end
  object GroupBox2: TGroupBox
    Left = 8
    Top = 224
    Width = 617
    Height = 273
    Caption = 'Status'
    TabOrder = 4
    object Status: TListBox
      Left = 8
      Top = 16
      Width = 601
      Height = 225
      ItemHeight = 13
      TabOrder = 0
    end
    object autoscroll: TCheckBox
      Left = 8
      Top = 248
      Width = 601
      Height = 17
      Caption = 'Enable Auto-Scroll'
      Checked = True
      State = cbChecked
      TabOrder = 1
    end
  end
  object Go: TButton
    Left = 272
    Top = 184
    Width = 97
    Height = 25
    Caption = 'Go!'
    TabOrder = 5
    OnClick = GoClick
  end
  object SaveDialog: TSaveDialog
    Filter = 'CSV File (*.csv)|*.csv'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofNoReadOnlyReturn, ofEnableSizing]
    Left = 592
    Top = 192
  end
end
