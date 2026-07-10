import type { CSSProperties } from 'react'
import { useStore } from '../state/store'
import { SPX_PROGS } from '../state/seed'
import { panelCard, paramLabel } from '../theme/tokens'
import { Knob } from '../components/Knob'
import { Editable } from '../components/Editable'

const dispChipSm: CSSProperties = { font: "500 11px 'IBM Plex Mono'", color: 'var(--disp-ink)', background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: 3, padding: '2px 10px', boxShadow: 'var(--disp-inset)', textAlign: 'center' }
const pad2 = (n: number) => (n < 10 ? '0' : '') + n

export function FxView() {
  const fx = useStore((s) => s.fx)
  const spx = useStore((s) => s.spx)
  const toggleFxBlock = useStore((s) => s.toggleFxBlock)
  const setFxKnob = useStore((s) => s.setFxKnob)
  const setSpxProgram = useStore((s) => s.setSpxProgram)
  const setSpxParam = useStore((s) => s.setSpxParam)

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: 10, padding: '12px 14px' }}>
      {/* FX500 simul-chain */}
      <div style={{ ...panelCard, padding: '11px 13px', display: 'flex', flexDirection: 'column', gap: 10 }}>
        <div style={{ display: 'flex', alignItems: 'baseline', gap: 10 }}>
          <div style={{ font: "700 14px 'Barlow Condensed'", letterSpacing: '.06em', color: 'var(--ink)' }}>FX500</div>
          <div style={{ font: "600 9px 'Barlow Condensed'", letterSpacing: '.14em', color: 'var(--silk)' }}>SIMUL-CHAIN · 5 BLOCKS IN SERIES</div>
        </div>
        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(5,1fr)', gap: 8 }}>
          {fx.map((b, bi) => (
            <div key={bi} style={{ display: 'flex', flexDirection: 'column', gap: 9, background: 'var(--inset)', border: '1px solid var(--panel-b)', borderRadius: 5, padding: '10px 10px 12px', opacity: b.enabled ? 1 : 0.5 }}>
              <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
                <div style={{ font: "700 12px 'Barlow Condensed'", letterSpacing: '.05em', color: 'var(--ink)' }}>{b.name}</div>
                <div onClick={() => toggleFxBlock(bi)} style={{ cursor: 'pointer', font: "600 8px 'Barlow Condensed'", letterSpacing: '.1em', padding: '2px 6px', borderRadius: 3, color: b.enabled ? 'var(--keyA-ink)' : 'var(--silk)', background: b.enabled ? 'var(--keyA)' : 'var(--panel)', border: '1px solid ' + (b.enabled ? 'var(--keyA-b)' : 'var(--panel-b)') }}>{b.enabled ? 'ON' : 'BYP'}</div>
              </div>
              <div style={{ display: 'flex', flexWrap: 'wrap', gap: 10, justifyContent: 'center', paddingTop: 2 }}>
                {b.knobs.map((k, ki) => (
                  <div key={ki} style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 4 }}>
                    <div style={{ font: "600 8px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--silk)' }}>{k.label}</div>
                    <Knob value={k.val} size={30} onChange={(v) => setFxKnob(bi, ki, v)} />
                    <Editable value={String(k.val)} onCommit={(v) => setFxKnob(bi, ki, parseInt(v) || 0)} style={{ font: "500 9px 'IBM Plex Mono'", color: 'var(--val)' }} />
                  </div>
                ))}
              </div>
            </div>
          ))}
        </div>
      </div>

      {/* SPX90 */}
      <div style={{ ...panelCard, padding: '11px 13px', display: 'grid', gridTemplateColumns: '230px 1fr', gap: 16 }}>
        <div style={{ display: 'flex', flexDirection: 'column', gap: 10 }}>
          <div style={{ display: 'flex', alignItems: 'baseline', gap: 8 }}>
            <div style={{ font: "700 14px 'Barlow Condensed'", letterSpacing: '.06em', color: 'var(--ink)' }}>SPX90</div>
            <div style={{ font: "600 9px 'Barlow Condensed'", letterSpacing: '.14em', color: 'var(--silk)' }}>ONE ALGORITHM</div>
          </div>
          <div style={{ display: 'flex', alignItems: 'center', gap: 12 }}>
            <div style={{ background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: 4, padding: '5px 14px', boxShadow: 'inset 0 1px 5px rgba(0,0,0,.3)' }}><div style={{ font: "600 26px 'IBM Plex Mono'", color: 'var(--disp-ink)' }}>{pad2(spx.program + 1)}</div></div>
            <div style={{ display: 'flex', flexDirection: 'column', gap: 2 }}><div style={{ font: "600 12px 'Barlow Condensed'", letterSpacing: '.06em', color: 'var(--ink)' }}>{SPX_PROGS[spx.program]}</div><div style={{ font: "400 9px 'IBM Plex Mono'", color: 'var(--faint)' }}>PROGRAM {pad2(spx.program + 1)} / 30</div></div>
          </div>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
            {SPX_PROGS.map((nm, i) => {
              const on = i === spx.program
              return (
                <div key={i} onClick={() => setSpxProgram(i)} style={{ display: 'flex', alignItems: 'center', gap: 8, padding: '5px 8px', borderRadius: 4, cursor: 'pointer', background: on ? 'var(--szl)' : 'transparent', border: '1px solid ' + (on ? 'var(--szl-b)' : 'transparent') }}>
                  <div style={{ font: "500 10px 'IBM Plex Mono'", color: 'var(--faint)', width: 22 }}>{pad2(i + 1)}</div>
                  <div style={{ font: "500 10.5px 'IBM Plex Mono'", color: on ? 'var(--acc)' : 'var(--val)' }}>{nm}</div>
                </div>
              )
            })}
          </div>
        </div>
        <div style={{ display: 'flex', flexDirection: 'column', gap: 10 }}>
          <div style={paramLabel}>HALL PARAMETERS</div>
          <div style={{ display: 'grid', gridTemplateColumns: 'repeat(5,1fr)', gap: 12 }}>
            {spx.params.map((k, i) => (
              <div key={i} style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 5 }}>
                <div style={{ font: "600 8.5px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--silk)', textAlign: 'center' }}>{k.label}</div>
                <Knob value={k.val} size={40} onChange={(v) => setSpxParam(i, v)} />
                <Editable value={String(k.val)} onCommit={(v) => setSpxParam(i, parseInt(v) || 0)} style={dispChipSm} />
              </div>
            ))}
          </div>
          <div style={{ font: "400 9px 'IBM Plex Mono'", color: 'var(--faint)', marginTop: 'auto' }}>MEP4 NOTE ECHO LIVES IN THE MIDI TAB — IT PROCESSES NOTES BEFORE THE SYNTH</div>
        </div>
      </div>
    </div>
  )
}
