import type { CSSProperties } from 'react'
import { useStore } from '../state/store'
import { auditionNote } from '../audio/useAudio'
import { requestSelectVoice } from '../plugin/bridge'
import { ALGACC } from '../theme/themes'
import { ALGS } from '../state/seed'
import { AlgorithmGlyph } from '../components/AlgorithmGlyph'
import { EnvelopeGraph, PitchEgGraph } from '../components/EnvelopeGraph'
import { Knob } from '../components/Knob'
import { Editable } from '../components/Editable'
import {
  dispChip, panelCard, sectionLabel, paramLabel, segOn, segOff, chipKey, chipKeyA,
  chipKeySm, chipKeyASm, keyBtn,
} from '../theme/tokens'

const chipKeyA2: CSSProperties = { font: "500 10px 'IBM Plex Mono'", color: 'var(--keyA-ink)', background: 'var(--keyA)', border: '1px solid var(--keyA-b)', borderRadius: 4, padding: '3px 8px' }
const chipDim: CSSProperties = { font: "500 10px 'IBM Plex Mono'", color: 'var(--silk)', border: '1px solid var(--panel-b)', borderRadius: 4, padding: '3px 8px' }
const roleTag = (isC: boolean): CSSProperties => ({ font: "600 9px 'Barlow Condensed'", letterSpacing: '.13em', padding: '2px 6px', borderRadius: 3, background: isC ? 'var(--szl)' : 'transparent', color: isC ? 'var(--acc)' : 'var(--silk)', border: '1px solid ' + (isC ? 'var(--szl-b)' : 'var(--panel-b)') })

export function VoiceView() {
  const theme = useStore((s) => s.theme)
  const mode = useStore((s) => s.mode)
  const patch = useStore((s) => s.patch)
  const bank = useStore((s) => s.bank)
  const setMode = useStore((s) => s.setMode)
  const setAlgorithm = useStore((s) => s.setAlgorithm)
  const setFeedback = useStore((s) => s.setFeedback)
  const setOpField = useStore((s) => s.setOpField)
  const setOpEnvelope = useStore((s) => s.setOpEnvelope)
  const setLfoKnob = useStore((s) => s.setLfoKnob)
  const audioStub = useStore((s) => s.audioStub)
  const A = ALGACC[theme]
  const tx = mode === 'TX81Z'

  return (
    <div>
      {/* sub-header: mode toggle · patch nav · compare */}
      <div style={{ display: 'flex', alignItems: 'center', gap: 12, padding: '10px 16px', borderBottom: '1px solid var(--head-b)' }}>
        <div style={{ display: 'flex', gap: 3, background: 'var(--panel)', border: '1px solid var(--panel-b)', borderRadius: 5, padding: 3 }}>
          <div style={{ ...(tx ? segOff : segOn), cursor: 'pointer' }} onClick={() => setMode('DX21')}>DX21</div>
          <div style={{ ...(tx ? segOn : segOff), cursor: 'pointer' }} onClick={() => setMode('TX81Z')}>TX81Z</div>
        </div>
        <div style={{ flex: 1 }} />
        <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
          <div onClick={() => { if (bank.loaded) requestSelectVoice(Math.max(0, bank.current - 1)); else { audioStub('prev patch'); auditionNote() } }} style={{ ...navBtn, cursor: 'pointer' }}>‹</div>
          <div style={{ display: 'flex', alignItems: 'center', gap: 12, background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: 4, padding: '6px 14px', boxShadow: 'inset 0 1px 5px rgba(0,0,0,.3)' }}>
            <div style={{ font: "500 12px 'IBM Plex Mono'", color: 'var(--disp-dim)' }}>{patch.slot}</div>
            <Editable value={patch.name} style={{ font: "500 14px 'IBM Plex Mono'", color: 'var(--disp-ink)' }} />
            {patch.edited && <div style={{ font: "500 9px 'IBM Plex Mono'", color: 'var(--disp-ink)', border: '1px solid var(--disp-b)', borderRadius: 3, padding: '1px 5px' }}>EDITED</div>}
          </div>
          <div onClick={() => { if (bank.loaded) requestSelectVoice(Math.min(bank.names.length - 1, bank.current + 1)); else { audioStub('next patch'); auditionNote() } }} style={{ ...navBtn, cursor: 'pointer' }}>›</div>
          <div onClick={() => audioStub('compare')} style={{ marginLeft: 6, padding: '6px 12px', background: 'var(--key)', border: '1px solid var(--key-b)', borderRadius: 4, font: "600 10.5px 'Barlow Condensed'", letterSpacing: '.12em', color: 'var(--key-ink)', boxShadow: '0 2px 3px rgba(0,0,0,.3)', cursor: 'pointer' }}>COMPARE</div>
        </div>
      </div>

      <div style={{ display: 'grid', gridTemplateColumns: '236px 1fr', gap: 10, padding: '12px 14px' }}>
        {/* left column */}
        <div style={{ display: 'flex', flexDirection: 'column', gap: 10 }}>
          <div style={{ ...panelCard, padding: '10px 12px 12px', display: 'flex', flexDirection: 'column', gap: 8 }}>
            <div style={{ display: 'flex', alignItems: 'baseline', justifyContent: 'space-between' }}>
              <div style={sectionLabel}>ALGORITHM</div>
              <div style={{ font: "500 11px 'IBM Plex Mono'", color: 'var(--acc)' }}>{patch.algorithm + 1}</div>
            </div>
            <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 6 }}>
              {ALGS.map((_, i) => {
                const sel = i === patch.algorithm
                const bs: CSSProperties = {
                  display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center', gap: 5, height: 62, borderRadius: 5, cursor: 'pointer',
                  background: sel ? A.selBg : A.unselBg, border: '1px solid ' + (sel ? A.selB : A.unselB),
                  boxShadow: sel ? ('inset 0 2px 5px rgba(0,0,0,.3)' + (A.glow !== 'none' ? ', ' + A.glow : '')) : 'inset 0 1px 0 rgba(255,255,255,.25), 0 2px 3px rgba(0,0,0,.35)',
                }
                return (
                  <div key={i} style={bs} onClick={() => setAlgorithm(i)}>
                    <AlgorithmGlyph index={i} selected={sel} A={A} />
                    <div style={{ font: "600 9px 'IBM Plex Mono'", color: sel ? A.ac : A.numUnsel }}>{i + 1}</div>
                  </div>
                )
              })}
            </div>
          </div>

          <div style={{ ...panelCard, padding: '10px 12px', display: 'flex', alignItems: 'center', gap: 14 }}>
            <Knob value={patch.feedback} min={0} max={7} size={44} pointerLen={17} onChange={setFeedback} />
            <div style={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
              <div style={sectionLabel}>FEEDBACK</div>
              <Editable value={String(patch.feedback)} onCommit={(v) => setFeedback(parseInt(v) || 0)} style={{ font: "500 13px 'IBM Plex Mono'", color: 'var(--ink)' }} />
              <div style={{ font: "400 9px 'IBM Plex Mono'", color: 'var(--faint)' }}>OP4 → OP4</div>
            </div>
          </div>

          <div style={{ ...panelCard, padding: '9px 12px', display: 'flex', gap: 12, alignItems: 'center' }}>
            <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}><div style={{ width: 11, height: 8, borderRadius: 2, background: 'var(--acc)' }} /><div style={{ font: "600 9px 'Barlow Condensed'", letterSpacing: '.12em', color: 'var(--silk)' }}>CARRIER</div></div>
            <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}><div style={{ width: 11, height: 8, borderRadius: 2, border: '1px solid var(--faint)' }} /><div style={{ font: "600 9px 'Barlow Condensed'", letterSpacing: '.12em', color: 'var(--silk)' }}>MODULATOR</div></div>
          </div>
        </div>

        {/* right column: operator strips + bottom row */}
        <div style={{ display: 'flex', flexDirection: 'column', gap: 8 }}>
          {patch.ops.map((op, i) => (
            <div key={i} style={{ display: 'grid', gridTemplateColumns: '148px 264px 1fr', gap: 14, alignItems: 'center', ...panelCard, padding: '9px 14px' }}>
              <div style={{ display: 'flex', flexDirection: 'column', gap: 7 }}>
                <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
                  <div style={{ font: "700 17px 'Barlow Condensed'", letterSpacing: '.06em', color: 'var(--ink)' }}>{op.name}</div>
                  <div style={roleTag(op.carrier)}>{op.role}</div>
                  <div style={{ width: 6, height: 6, borderRadius: '50%', background: 'var(--acc)', boxShadow: '0 0 6px var(--accg)' }} />
                </div>
                <div style={{ display: 'flex', gap: 5, opacity: tx ? 1 : 0.35 }}>
                  <div style={tx ? chipKeyA2 : chipDim}>{op.wave}</div>
                  <div style={chipDim}>{tx ? 'FIX OFF' : 'TX ONLY'}</div>
                </div>
              </div>
              <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr 1fr', gap: 10 }}>
                {([['RATIO', 'ratio'], ['LEVEL', 'lvl'], ['DETUNE', 'det']] as const).map(([label, field]) => (
                  <div key={field} style={{ display: 'flex', flexDirection: 'column', gap: 4 }}>
                    <div style={paramLabel}>{label}</div>
                    <Editable value={op[field]} onCommit={(v) => setOpField(i, field, v)} style={dispChip} />
                  </div>
                ))}
              </div>
              <div style={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'baseline' }}>
                  <div style={paramLabel}>ENVELOPE</div>
                  <div style={{ font: "400 9.5px 'IBM Plex Mono'", color: 'var(--faint)' }}>AR {op.eg}</div>
                </div>
                <EnvelopeGraph ep={op.ep} w={380} h={58} onChange={(ep) => setOpEnvelope(i, ep)} />
              </div>
            </div>
          ))}

          {/* bottom row */}
          <div style={{ display: 'grid', gridTemplateColumns: '1.25fr 0.95fr 1.35fr 1.05fr', gap: 8 }}>
            {/* LFO */}
            <div style={{ ...panelCard, padding: '9px 12px', display: 'flex', flexDirection: 'column', gap: 8 }}>
              <div style={sectionLabel}>LFO</div>
              <div style={{ display: 'flex', alignItems: 'flex-start', gap: 12 }}>
                <div style={{ display: 'flex', flexDirection: 'column', gap: 5 }}>
                  <div style={chipKey}>{patch.lfo.wave}</div>
                  <div style={chipKeyA}>SYNC</div>
                </div>
                {patch.lfo.knobs.map((k, ki) => (
                  <div key={k.key} style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 4 }}>
                    <div style={{ font: "600 9px 'Barlow Condensed'", letterSpacing: '.12em', color: 'var(--silk)' }}>{k.label}</div>
                    <Knob value={k.val} size={32} onChange={(v) => setLfoKnob(ki, v)} />
                    <Editable value={String(k.val)} onCommit={(v) => setLfoKnob(ki, parseInt(v) || 0)} style={{ font: "500 10px 'IBM Plex Mono'", color: 'var(--val)' }} />
                  </div>
                ))}
              </div>
            </div>

            {/* Pitch EG */}
            <div style={{ ...panelCard, padding: '9px 12px', display: 'flex', flexDirection: 'column', gap: 6 }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'baseline' }}><div style={sectionLabel}>PITCH EG</div><div style={{ font: "400 9.5px 'IBM Plex Mono'", color: 'var(--faint)' }}>{patch.pitchEg.label}</div></div>
              <PitchEgGraph w={280} h={52} />
            </div>

            {/* Function */}
            <div style={{ ...panelCard, padding: '9px 12px', display: 'flex', flexDirection: 'column', gap: 8 }}>
              <div style={sectionLabel}>FUNCTION</div>
              <div style={{ display: 'flex', gap: 10, alignItems: 'center', flexWrap: 'wrap' }}>
                <div style={{ display: 'flex', gap: 3 }}><div style={patch.fn.voice === 'POLY' ? segOn : segOff}>POLY</div><div style={patch.fn.voice === 'MONO' ? segOn : segOff}>MONO</div></div>
                <div style={{ display: 'flex', flexDirection: 'column', gap: 1 }}><div style={{ font: "600 8.5px 'Barlow Condensed'", letterSpacing: '.12em', color: 'var(--silk)' }}>PORTA</div><div style={{ font: "500 12px 'IBM Plex Mono'", color: 'var(--ink)' }}>{patch.fn.porta}</div></div>
                <div style={{ display: 'flex', flexDirection: 'column', gap: 1 }}><div style={{ font: "600 8.5px 'Barlow Condensed'", letterSpacing: '.12em', color: 'var(--silk)' }}>TRANSP</div><div style={{ font: "500 12px 'IBM Plex Mono'", color: 'var(--ink)' }}>{patch.fn.transpose >= 0 ? '+' : '−'}{Math.abs(patch.fn.transpose)}</div></div>
                <div style={{ display: 'flex', gap: 3 }}>{(['SNGL', 'DUAL', 'SPLIT'] as const).map((a) => <div key={a} style={patch.fn.assign === a ? segOn : segOff}>{a}</div>)}</div>
              </div>
            </div>

            {/* Sizzler */}
            <div style={{ background: 'var(--szl)', border: '1px solid var(--szl-b)', borderRadius: 6, padding: '9px 12px', display: 'flex', flexDirection: 'column', gap: 6 }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'baseline' }}><div style={{ font: "600 10px 'Barlow Condensed'", letterSpacing: '.16em', color: 'var(--szl-t)' }}>SIZZLER</div><div style={{ font: "400 9px 'IBM Plex Mono'", color: 'var(--faint)' }}>{patch.sizzler.targets} TARGETS</div></div>
              <div style={{ display: 'flex', alignItems: 'center', gap: 12 }}>
                <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 3 }}>
                  <Knob value={patch.sizzler.amount} size={32} borderColor="var(--szl-b)" pointerColor="var(--szl-t)" />
                  <div style={{ font: "500 10px 'IBM Plex Mono'", color: 'var(--val)' }}>{patch.sizzler.amount}</div>
                </div>
                <div style={{ display: 'flex', flexDirection: 'column', gap: 4 }}>
                  <div style={{ display: 'flex', gap: 3 }}>{(['RND', 'MRF', 'DRF'] as const).map((m) => <div key={m} style={patch.sizzler.mode === m ? chipKeyASm : chipKeySm}>{m}</div>)}</div>
                  <div style={{ ...keyBtn, alignSelf: 'flex-start', padding: '3px 10px', font: "600 9.5px 'Barlow Condensed'", letterSpacing: '.12em' }}>FREEZE</div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}

const navBtn: CSSProperties = { width: 26, height: 26, display: 'flex', alignItems: 'center', justifyContent: 'center', background: 'var(--key)', border: '1px solid var(--key-b)', borderRadius: 4, color: 'var(--key-ink)', font: "600 12px 'IBM Plex Mono'", boxShadow: '0 2px 3px rgba(0,0,0,.3)' }
